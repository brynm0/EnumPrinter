/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Bryn Murrell $
   ======================================================================== */
#include "blib_utils.h"
#include <cstdio>
#include <string>
#include "Parsing.h"
#include <vector>
#include "len_string.h"
#include "DebugFileIO.cpp"

flocal void add_enum_function_to_string(len_string* h_file,
                                              char** enum_start_string)
{
    Tokenizer tok = {};
    tok.at = *enum_start_string;
    Token enum_name = getToken(&tok);
    b32 should_break = false;
    u32 running_value = 0;
    struct token_value_pair
    {
        Token identifier;
        u32 value;
    };
    std::vector<token_value_pair> all_enum_identifier_values = {};
    while (!should_break)
    {
        Token t = getToken(&tok);
        switch (t.type)
        {
            case TOKEN_END :
            {
                ASSERT(1==0, "Unexpected EOF found in enum");
            } break;
            case TOKEN_IDENTIFIER :
            {
                
                //store token id & value in vector
                token_value_pair pair = {};
                pair.identifier = t;
                Token next_tok = getToken(&tok);
                if (next_tok.type == TOKEN_COMMA || next_tok.type == TOKEN_BRACE_CLOSE)
                {
                    if (next_tok.type == TOKEN_BRACE_CLOSE)
                    {
                        should_break = true;
                    }
                    //TODO verify if enums behave this way
                    pair.value = running_value++;
                }
                else if (next_tok.type == TOKEN_ASSIGNMENT)
                {
                    
                    next_tok = getToken(&tok);
                    char* end = (next_tok.text + next_tok.length);
                    long int number = strtol(next_tok.text,
                                             &end,
                                             10);
                    ASSERT(next_tok.type == TOKEN_NUMBER, "Next token must be a number");
                    if (peek_tok(tok).type == TOKEN_LSHIFT ||
                        peek_tok(tok).type == TOKEN_RSHIFT)
                    {
                        Token shift_sign = getToken(&tok);
                        Token next_number = getToken(&tok);
                        end = next_number.text + next_number.length;
                        long int shifter = strtol(next_number.text,
                                                  &end,
                                                  10);
                        if (shift_sign.type == TOKEN_LSHIFT)
                        {
                            
                            pair.value = number << shifter;       
                        }
                        else if (shift_sign.type == TOKEN_RSHIFT)
                        {
                            pair.value = number >> shifter;
                        }
                                                  
                    }
                    else if (peek_tok(tok).type == TOKEN_COMMA)
                    {
                        pair.value = number;
                    }
                    else
                    {
                        ASSERT(1==0, "Expression in enum must be of supported type");
                    }
                }
                all_enum_identifier_values.push_back(pair);
//check for token comma & close brace + semicolon
                
            } break;
            case TOKEN_BRACE_CLOSE :
            {
                should_break = true;
            } break;
        }
    }

    char enum_name_null_terminated_array[256];
    char* enum_name_null_terminated = &enum_name_null_terminated_array[0];
    sub_str_to_null_terminated(enum_name.text, enum_name.length, (char**)&enum_name_null_terminated);
    
    append_to_len_string(h_file, "\n");
    char temp[256];
    sprintf((char*)temp,
            "flocal len_string print_enum_name_%s(u32 field)\n",
            enum_name_null_terminated,enum_name_null_terminated);
    append_to_len_string(h_file, (char*)temp);
    append_to_len_string(h_file, "{\n");
    append_to_len_string(h_file, "\tlen_string s = l_string(256);\n");
    for (int i = 0; i < all_enum_identifier_values.size(); i++)
    {
        char field_null_terminated_array[256];
        char* field_null_terminated = &field_null_terminated_array[0];
        sub_str_to_null_terminated(all_enum_identifier_values[i].identifier.text,
                                   all_enum_identifier_values[i].identifier.length,
                                   (char**)&field_null_terminated);
        
        sprintf((char*)temp,"\tif ( %s & field )\n\t{\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        sprintf((char*)temp,"\t\tappend_to_len_string(&s, \"%s\\n\");\n\t}\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        
    }

    append_to_len_string(h_file, "\treturn s;\n");
    append_to_len_string(h_file, "}\n");
    
    sprintf((char*)temp,
            "flocal len_string print_enum_name_%s_using_equals(u32 field)\n",
            enum_name_null_terminated,enum_name_null_terminated);
    append_to_len_string(h_file, (char*)temp);
    append_to_len_string(h_file, "{\n");
    append_to_len_string(h_file, "\tlen_string s = l_string(256);\n");
    for (int i = 0; i < all_enum_identifier_values.size(); i++)
    {
        char field_null_terminated_array[256];
        char* field_null_terminated = &field_null_terminated_array[0];
        sub_str_to_null_terminated(all_enum_identifier_values[i].identifier.text,
                                   all_enum_identifier_values[i].identifier.length,
                                   (char**)&field_null_terminated);
        
        sprintf((char*)temp,"\tif ( %s == field )\n\t{\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        sprintf((char*)temp,"\t\tappend_to_len_string(&s, \"%s\");\n\t}\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        
    }
    append_to_len_string(h_file, "\treturn s;\n");
    append_to_len_string(h_file, "}\n\n");
    
    sprintf((char*)temp,
            "flocal u32 %s_name_to_bits(char* name)\n",
            enum_name_null_terminated);
    append_to_len_string(h_file, (char*)temp);
    append_to_len_string(h_file, "{\n");
    for (int i = 0; i < all_enum_identifier_values.size(); i++)
    {
        char field_null_terminated_array[256];
        char* field_null_terminated = &field_null_terminated_array[0];
        sub_str_to_null_terminated(all_enum_identifier_values[i].identifier.text,
                                   all_enum_identifier_values[i].identifier.length,
                                   (char**)&field_null_terminated);
        
        sprintf((char*)temp,"\tif ( streq(\"%s\", name, strlen(name)) )\n\t{\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        sprintf((char*)temp,"\t\treturn (u32)%s;\n\t}\n", field_null_terminated);
        append_to_len_string(h_file, (char*)temp);
        
    }

    append_to_len_string(h_file, "\treturn -1;\n");
    append_to_len_string(h_file, "}\n\n");
    
    sprintf((char*)temp,
            "global_variable char* %s_name_arr[] = \n",
            enum_name_null_terminated);
    append_to_len_string(h_file, (char*)temp);
    append_to_len_string(h_file, "{\n");
    for (int i = 0; i < all_enum_identifier_values.size(); i++)
    {
        char field_null_terminated_array[256];
        char* field_null_terminated = &field_null_terminated_array[0];
        sub_str_to_null_terminated(all_enum_identifier_values[i].identifier.text,
                                   all_enum_identifier_values[i].identifier.length,
                                   (char**)&field_null_terminated);
        if (i != all_enum_identifier_values.size()-1)
        {
            sprintf((char*)temp,"\t\"%s\",\n", field_null_terminated);
        }
        else
        {
            sprintf((char*)temp,"\t\"%s\"\n", field_null_terminated);
        }
        append_to_len_string(h_file, (char*)temp);
        
    }

    append_to_len_string(h_file, "};\n\n");
    
    sprintf((char*)temp,
            "global_variable u32 %s_val_arr[] = \n",
            enum_name_null_terminated);
    append_to_len_string(h_file, (char*)temp);
    append_to_len_string(h_file, "{\n");
    for (int i = 0; i < all_enum_identifier_values.size(); i++)
    {
        char field_null_terminated_array[256];
        char* field_null_terminated = &field_null_terminated_array[0];
        sub_str_to_null_terminated(all_enum_identifier_values[i].identifier.text,
                                   all_enum_identifier_values[i].identifier.length,
                                   (char**)&field_null_terminated);
        if (i != all_enum_identifier_values.size()-1)
        {
            sprintf((char*)temp,"\t%d,\n", all_enum_identifier_values[i].value);
        }
        else
        {
            sprintf((char*)temp,"\t%d\n", all_enum_identifier_values[i].value);
        }
        append_to_len_string(h_file, (char*)temp);
        
    }


    append_to_len_string(h_file, "};\n\n");
    
    sprintf((char*)temp,
            "global_variable u32 %s_field_count = %lld;\n",
            enum_name_null_terminated,
            all_enum_identifier_values.size());
    append_to_len_string(h_file, (char*)temp);
    
//TODO: finish this
}
                                        

int main (int argc, char** argv)
{
    if (!(argc < 2) && is_cpp_or_h_file(argv[1]))
    {
        //printf("cpp or h file passed\n");
    }
    else
    {
        //printf("cpp or h file passed\n");
    }
    u64 length = 0;
    char* full_file_str = read_entire_file_text(argv[1], &length);

    len_string inl_string = l_string(256);     
         
    char* c = argv[1];
    while (*c != '.')
    {
        c++;
    }
    u32 file_name_no_ext_len = c - argv[1];    
    while (full_file_str)
    {
        b32 found_enum = find_next_enum_in_file(&full_file_str);
        if (found_enum)
        {
            add_enum_function_to_string(&inl_string,
                                        &full_file_str);
            //full_file_str = parse_enum(&inl_string, full_file_str);
        }
        else
        {
            break;
        }
    }
    char file_name_no_ext[256];
    strcpy(file_name_no_ext,
        argv[1]);
    file_name_no_ext[file_name_no_ext_len] = 0;
    len_string l = l_string(256);
    append_to_len_string(&l, file_name_no_ext);
    append_to_len_string(&l, ".inl");
    FILE * out_inl = fopen(l.str, "w");
    fprintf(out_inl, inl_string.str);
    fclose(out_inl);

    return 0;
            
    
}
