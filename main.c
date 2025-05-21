#include <stdio.h>
#include "types.h"
#include "view_mp3.h"
#include "edit_mp3.h"
#include "string.h"

int view_flag, edit_flag;

int main(int argc, char *argv[])
{
    TagInfo taginfo;
    TagData tagdata;
    
    // Check if enough arguments are provided
    if (argc < 2)
    {
        printf("ERROR : INCORRECT FORMAT\n");
        printf("INFO : USE --help FOR HELP MENU\n");
        return 1;
    }
    
    // Determine operation type
    if (check_operation_type(argv) == view)
    {
        printf("--------------------------------INFO : SELECTED VIEW MODE-----------------------------------------\n\n");
        printf("--------------------------------------------------------------------------------------------------\n");
        printf("\t\t\t\tMP3 TAG READER AND EDITOR\n");
        printf("--------------------------------------------------------------------------------------------------\n");
        
        // Validate input file and retrieve tag info
        if (read_and_validate(argv, &taginfo) == success)
        {
            printf("INFO : READ AND VALIDATE SUCCESS\n");
            if (get_info(&taginfo) == success)
            {
                printf("INFO : TAGS READ SUCCESSFULLY\n");
                printf("-----------------------------INFO : DISPLAYING DATA------------------------------------------------\n");
                if (display_data(&taginfo) == success)
                {
                    printf("-----------------------------INFO : DISPLAYED DATA SUCCESSFULLY-------------------------------------\n\n");
                }
                else
                {
                    printf("ERROR : DISPLAY DATA FAILED\n");
                    return 1;
                }
            }
            else
            {
                printf("ERROR : GETTING INFO FAILED \n");
            }
        }
        else
        {
            printf("ERROR : READ AND VALIDATE FAILED\n");
        }
    }
    else if (check_operation_type(argv) == edit)
    {
        printf("--------------------------------INFO : SELECTED EDIT MODE-----------------------------------------\n\n");
        printf("--------------------------------------------------------------------------------------------------\n");
        printf("\t\t\t\tMP3 TAG READER AND EDITOR\n");
        printf("--------------------------------------------------------------------------------------------------\n");
        
        // Validate edit operation parameters
        if (read_and_validate_edit(argv, &tagdata) == success)
        {
            printf("INFO : READ AND VALIDATE SUCCESS\n");
            taginfo.fname = argv[4]; // Assign filename for editing
            get_info(&taginfo);
            
            printf("-----------------------------INFO : EDITING DATA ------------------------------------------------\n");
            if (edit_data(&tagdata, &taginfo) == success)
            {
                printf("-----------------------------INFO : EDITED DATA SUCCESSFULLY-------------------------------------\n\n");
            }
            else
            {
                printf("ERROR : EDIT DATA FAILED\n");
            }
        }
        else
        {
            printf("ERROR : READ AND VALIDATE FAILED \n");
            return 1;
        }
    }
    else if (check_operation_type(argv) == help)
    {
        // Display help menu
        printf("INFO : SELECTED HELP MENU\n");
        printf("-------------------------------------------------------------------------------------------------------\n\n");
        printf("---------------------------------------------HELP MENU-------------------------------------------------\n\n");
        printf("-------------------------------------------------------------------------------------------------------\n");
        printf("INFO: Help Menu for Tag Reader and Editor:\n");
        printf("INFO: For Viewing the Tags -> ./mp3_tag_reader -v <file_name.mp3>\n");
        printf("INFO: For Editing the Tags -> ./mp3_tag_reader -e <modifier> \"New_Value\" <file_name.mp3>\n");
        printf("INFO: Modifier Functions:\n");
        printf("-t\tModify Title Tag\n-a\tModify Artist Tag\n-A\tModify Album Tag\n-y\tModify Year Tag\n-G\tModify Content Type Tag\n-c\tModify Comments Tag\n");
        printf("-------------------------------------------------------------------------------------------------------\n");
    }
    else if (check_operation_type(argv) == unsupported)
    {
        printf("ERROR : INCORRECT FORMAT\n");
        printf("INFO : USE --help FOR HELP MENU\n"); 
    }
}