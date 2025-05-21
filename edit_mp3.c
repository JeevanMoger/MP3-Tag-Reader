#include <stdio.h>
#include <string.h>
#include "edit_mp3.h"
#include "view_mp3.h"
#include "types.h"

char buf[3];
char tag_buf[4];
uint size_edit;
int title_flag,art_flag,alb_flag,y_flag,m_flag,c_flag;

/**
 * Function to read and validate the edit operation on MP3 metadata.
 * It verifies the command-line arguments, determines which tag to edit,
 * processes the tag data, and checks if the provided file is a valid MP3.
 */
Status read_and_validate_edit(char *argv[], TagData *tagdata)
{
	// Check if sufficient arguments are provided
	if(argv[2] == NULL || argv[3] == NULL || argv[4] == NULL)
	{
		printf("ERROR : INSUFFICIENT ARGUMENTS \n");
		printf("INFO : USE --help Command for help menu\n");
		return failure;
	}
	else
	{
		// Check which tag needs to be edited based on the argument
		if(strcmp(argv[2],"-t") == 0)  // Edit Title Tag
		{
			strcpy(tagdata->tag_Id,"TIT2");
			title_flag = 1;
		}
		else if(strcmp(argv[2],"-a") == 0)  // Edit Artist Tag
		{
			strcpy(tagdata->tag_Id,"TPE1");
			printf("INFO : EDITING ARTIST TAG\n");
			printf("TAG TO BE EDITED %s\n",tagdata -> tag_Id);
			art_flag = 1;
		}
		else if(strcmp(argv[2],"-A") == 0)  // Edit Album Tag
		{
			strcpy(tagdata->tag_Id,"TALB");
			printf("INFO : EDITING ALBUM TAG\n");
			printf("TAG TO BE EDITED %s\n",tagdata -> tag_Id);
			alb_flag = 1;
		}
		else if(strcmp(argv[2],"-y") == 0)  // Edit Year Tag
		{
			strcpy(tagdata->tag_Id,"TYER");
			printf("INFO : EDITING YEAR TAG\n");
			printf("TAG TO BE EDITED %s\n",tagdata -> tag_Id);
			y_flag = 1;
		}
		else if(strcmp(argv[2],"-m") == 0)  // Edit Music Genre Tag
		{
			strcpy(tagdata->tag_Id,"TCON");
			printf("INFO : EDITING CONTENT TAG\n");
			printf("TAG TO BE EDITED %s\n",tagdata -> tag_Id);
			m_flag = 1;
		}
		else if(strcmp(argv[2],"-c") == 0)  // Edit Comments Tag
		{
			strcpy(tagdata->tag_Id,"COMM");
			printf("INFO : EDITING COMMENT TAG\n");
			printf("INFO : TAG TO BE EDITED %s\n",tagdata -> tag_Id);
			c_flag = 1;
		}

		// Store the new tag data to be written
		strcpy(tagdata -> tag_data, argv[3]);
		printf("INFO : DATA TO BE EDITED IS %s\n", tagdata -> tag_data);

		// Calculate and convert the size of the new tag data to little-endian format
		size_edit = strlen(tagdata -> tag_data) + 1;  // Including null-terminator
		big_to_little(&size_edit);
		tagdata -> data_size = size_edit;
		printf("INFO : SIZE OF DATA TO BE EDITED %d\n", tagdata -> data_size);

		// Store the file name
		strcpy(tagdata->fname, argv[4]);
		printf("INFO : FILE NAME IS %s\n", tagdata -> fname);

		// Open the file to check its validity
		tagdata -> fdata = fopen(tagdata -> fname, "r");
		if(tagdata -> fdata == NULL)
		{
			printf("ERROR : FILE COULD NOT BE OPENED \n");
			return failure;
		}

		// Read the first 3 bytes of the file to check if it's a valid MP3 file
		fread(tag_buf, 1, 3, tagdata -> fdata);
		if(strcmp(tag_buf, "ID3") != 0)  // MP3 files with ID3 metadata start with "ID3"
		{
			printf("ERROR : PROVIDE A VALID MP3 FILE\n");
			return failure;
		}

		// Close the file as validation is complete
		fclose(tagdata -> fdata);
	}

	return success;
}

/**
 * Copies the remaining image data from fptr_src to fptr_dest.
 * Reads one byte at a time and writes it to the destination file.
 * Returns success upon completion.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
	char ch;  // Variable to hold one byte of data

	// Read one byte at a time from fptr_src and write it to fptr_dest
	while(fread(&ch, 1, 1, fptr_src) > 0)
	{
		fwrite(&ch, 1, 1, fptr_dest);
	}

	return success;
}


Status edit_data(TagData *tagdata,TagInfo *taginfo)
{
	char str[4];

	// Open the original file for reading and a temporary file for writing
	tagdata->fdata = fopen(tagdata->fname, "r");
	tagdata->ftemp = fopen("temp.mp3", "w");
	if (tagdata->fdata == NULL || tagdata->ftemp == NULL)
	{
		printf("ERROR : UNABLE TO OPEN THE REQUIRED FILES\n");
		return failure;
	}

	char buffer[10];
	fread(buffer, 1, 10, tagdata->fdata); // Read first 10 bytes (ID3 header)
	fwrite(buffer, 1, 10, tagdata->ftemp); // Write to temporary file

	if(title_flag)  // Check if the title tag needs to be edited
	{
		printf("INFO : EDITING TITLE TAG\n");

		// Move file pointer to the title tag position in the original file
		fseek(tagdata -> fdata, taginfo -> title_pos, SEEK_SET);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : TITLE TAG NOT FOUND \n");
			return failure; // Return failure if the title tag is not found
		}

		printf("INFO : TITLE TAG FOUND\n\n");
		printf("\t\tTITLE : %s\n\n", tagdata -> tag_data); // Print the new title data

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Move file pointer to the artist tag position to copy remaining data
		fseek(tagdata -> fdata, taginfo -> artist_pos, SEEK_SET);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the title tag
	}

	else if(art_flag) // Check if the artist tag needs to be edited
	{
		printf("INFO : EDITING ARTIST TAG\n");

		char ch;

		// Copy data from the original file up to the artist tag position
		while(fread(&ch, 1, 1, tagdata -> fdata) > 0)
		{
			int ptr = ftell(tagdata -> fdata); // Get the current file position

			// Stop copying if the current position is right before the artist tag
			if(ptr == (taginfo -> artist_pos + 1))
				break;

			fwrite(&ch, 1, 1, tagdata -> ftemp); // Write the byte to the temporary file
		}

		// Move file pointer back by one byte to correctly read the tag identifier
		fseek(tagdata -> fdata, -1, SEEK_CUR);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected artist tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : ARTIST TAG NOT FOUND \n");
			return failure; // Return failure if the artist tag is not found
		}

		printf("INFO : ARTIST TAG FOUND\n");
		printf("\t\tARTIST : %s\n\n", tagdata -> tag_data); // Print the new artist name

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Move file pointer to the album tag position to copy remaining data
		fseek(tagdata -> fdata, taginfo -> album_pos, SEEK_SET);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the artist tag
	}

	else if(alb_flag) // Check if the album tag needs to be edited
	{
		printf("INFO : EDITING ALBUM TAG\n");

		char ch;

		// Copy data from the original file up to the album tag position
		while(fread(&ch, 1, 1, tagdata -> fdata) > 0)
		{
			int ptr = ftell(tagdata -> fdata); // Get the current file position

			// Stop copying if the current position is right before the album tag
			if(ptr == (taginfo -> album_pos + 1))
				break;

			fwrite(&ch, 1, 1, tagdata -> ftemp); // Write the byte to the temporary file
		}

		// Move file pointer back by one byte to correctly read the tag identifier
		fseek(tagdata -> fdata, -1, SEEK_CUR);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected album tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : ALBUM TAG NOT FOUND \n");
			return failure; // Return failure if the album tag is not found
		}

		printf("INFO : ALBUM TAG FOUND\n");
		printf("\t\tTITLE : %s\n\n", tagdata -> tag_data); // Print the new album title

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little-endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Move file pointer to the year tag position to copy remaining data
		fseek(tagdata -> fdata, taginfo -> year_pos, SEEK_SET);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the album tag
	}

	else if(y_flag) // Check if the year tag needs to be edited
	{
		printf("INFO : EDITING YEAR TAG\n");

		char ch;

		// Copy data from the original file up to the year tag position
		while(fread(&ch, 1, 1, tagdata -> fdata) > 0)
		{
			int ptr = ftell(tagdata -> fdata); // Get the current file position

			// Stop copying if the current position is right before the year tag
			if(ptr == (taginfo -> year_pos + 1))
				break;

			fwrite(&ch, 1, 1, tagdata -> ftemp); // Write the byte to the temporary file
		}

		// Move file pointer back by one byte to correctly read the tag identifier
		fseek(tagdata -> fdata, -1, SEEK_CUR);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected year tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : YEAR TAG NOT FOUND \n");
			return failure; // Return failure if the year tag is not found
		}

		printf("INFO : YEAR TAG FOUND\n");
		printf("\t\tYEAR : %s\n\n", tagdata -> tag_data); // Print the new year value

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little-endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Move file pointer to the comment tag position to copy remaining data
		fseek(tagdata -> fdata, taginfo -> cont_pos, SEEK_SET);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the year tag
	}

	else if(m_flag) // Check if the music content tag needs to be edited
	{
		printf("INFO : EDITING CONTENT TAG\n");

		char ch;

		// Copy data from the original file up to the content tag position
		while(fread(&ch, 1, 1, tagdata -> fdata) > 0)
		{
			int ptr = ftell(tagdata -> fdata); // Get the current file position

			// Stop copying if the current position is right before the content tag
			if(ptr == (taginfo -> cont_pos + 1))
				break;

			fwrite(&ch, 1, 1, tagdata -> ftemp); // Write the byte to the temporary file
		}

		// Move file pointer back by one byte to correctly read the tag identifier
		fseek(tagdata -> fdata, -1, SEEK_CUR);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected music tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : MUSIC TAG NOT FOUND \n");
			return failure; // Return failure if the music tag is not found
		}

		printf("INFO : MUSIC TAG FOUND\n");
		printf("\t\tMUSIC : %s\n\n", tagdata -> tag_data); // Print the new music content value

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little-endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Move file pointer to the next tag position (comment tag) to copy remaining data
		fseek(tagdata -> fdata, taginfo -> comm_pos, SEEK_SET);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the content tag
	}

	else if(c_flag) // Check if the comment tag needs to be edited
	{
		printf("INFO : EDITING COMMENT TAG\n");

		char ch;

		// Copy data from the original file up to the comment tag position
		while(fread(&ch, 1, 1, tagdata -> fdata) > 0)
		{
			int ptr = ftell(tagdata -> fdata); // Get the current file position

			// Stop copying if the current position is right before the comment tag
			if(ptr == (taginfo -> comm_pos + 1))
				break;

			fwrite(&ch, 1, 1, tagdata -> ftemp); // Write the byte to the temporary file
		}

		// Move file pointer back by one byte to correctly read the tag identifier
		fseek(tagdata -> fdata, -1, SEEK_CUR);

		// Read the first 4 bytes (expected to be the tag identifier)
		fread(tag_buf, 1, 4, tagdata -> fdata);

		// Check if the tag identifier matches the expected comment tag ID
		if(strcmp(tag_buf, tagdata -> tag_Id) != 0)
		{
			printf("ERROR : COMMENT TAG NOT FOUND \n");
			return failure; // Return failure if the comment tag is not found
		}

		printf("INFO : COMMENT TAG FOUND\n");
		printf("\t\tCOMMENT : %s\n\n", tagdata -> tag_data); // Print the new comment value

		// Write the tag identifier to the temporary file
		fwrite(tagdata -> tag_Id, 1, 4, tagdata -> ftemp);

		// Read the next 3 bytes (typically flag or metadata info)
		fread(buf, 1, 3, tagdata -> fdata);

		// Write the data size field (converted to little-endian later)
		fwrite(&(tagdata -> data_size), 1, 4, tagdata -> ftemp);

		// Write back the 3 bytes read earlier
		fwrite(buf, 1, 3, tagdata -> ftemp);

		// Convert the data size from big-endian to little-endian format
		big_to_little(&(tagdata -> data_size));

		// Write the actual tag data (excluding the last byte)
		fwrite(tagdata -> tag_data, 1, (tagdata -> data_size) - 1, tagdata -> ftemp);

		// Skip over the existing comment tag size in the original file
		fseek(tagdata -> fdata, taginfo -> comment_size, SEEK_CUR);

		// Copy the remaining data (including any embedded images) from the original file
		copy_remaining_img_data(tagdata -> fdata, tagdata -> ftemp);

		// Close both files
		fclose(tagdata -> fdata);
		fclose(tagdata -> ftemp);

		// Remove the original file
		remove(tagdata -> fname);

		// Rename the temporary file to the original file name
		rename("temp.mp3", tagdata->fname);

		return success; // Return success after editing the comment tag
	}

}