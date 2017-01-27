#include <stdio.h>
#include <stdlib.h>
#ifdef __MSDOS__
#include "..\..\MT2D\Window_core.h"
#include "..\..\MT2D\Io_FILE.h"
#include "..\..\MT2D\Keyboard.h"
#include "..\..\MT2D\building_functions\display_popup.h"
#else
#include "../Window_core.h"
#include "../Io_FILE.h"
#include "../Keyboard.h"
#include "display_popup.h"
#endif
#include <string.h>

extern unsigned char WINDOW1[MAX_VER+1][MAX_HOR];
extern unsigned char WINDOW2[MAX_VER+1][MAX_HOR];

void transfer_files(char *files, char *destination){
	FILE *in;
	FILE *out;
	//int buffer; not used
	char number_percent[5]={' ',' ',' ','%%'};
	char percent_box[71]={176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,176,32,'\0'};
	char *temp_name;//Filter each name from files one by one
	int temp_name_size=0;
	long file_size=0;
	char *temp_destination;
	char buff;//Buffer to get char
	int pos_hor=4,pos_ver=10;//
	int win_hor=3,win_ver=9;// Position where the window will start to be
	int file_in=strlen(files);
	int file_out=strlen(destination);
	int j=0,i=0,k=0,l=0;
	float z=0;
	int percent_total=1;
	int total_files=0;
	int last_pointer=1;//Receive the last part of the filename stoped
	bool out_loop=false;
	number_percent[4]='\0';
	while(i<MAX_VER-1	){
		while(j<MAX_HOR){
			WINDOW2[i][j]=WINDOW1[i][j];
			j++;
		}
		i++;
		j=0;
	}
	i=0;
	while(i<=file_in){//Count how many files will be transfered
		if(files[i]=='*') total_files++;
		i++;
	}
	if(total_files>0 && check_adress_basic_test(destination)){
		temp_destination=(char*)malloc(file_out*sizeof(char));
		for(i=0;i<=file_out;i++){
			temp_destination[i]=destination[i];
		}
		while(percent_total<=total_files){
			create_window_layout(pos_hor,pos_ver,73,8,true,true,false,' ',DISPLAY_WINDOW2);
			insert_string_on_display("TRANSFERIR files V1.0",pos_ver+1,pos_hor+2,DISPLAY_WINDOW2);
			insert_string_on_display("(ENTER=CANCEL)",pos_ver+6,pos_hor+47,DISPLAY_WINDOW2);
			for(k=last_pointer;k<file_in && files[k]!='*';k++);
			temp_name_size=k-last_pointer+1;
			temp_name=(char*)malloc(temp_name_size*sizeof(char));
			if(!temp_name){
				printf("ERROR TO ALLOC NAME\n");
				Keyboard_keytouched(); // KEY WAIT LOCK
			}
			for(j=0;last_pointer<k;j++,last_pointer++){//Transfer just one filename
				temp_name[j]=files[last_pointer];
			}
			temp_name[j]='\0';
			insert_string_on_display(temp_name,pos_ver+4,pos_hor+2,DISPLAY_WINDOW2);
			last_pointer=k+1;
			i=j-1;
			for(i;i>0 && temp_name[i]!=92;i--);{
				i;
			}
			i++;
			temp_destination=(char*)realloc(temp_destination,(file_out+temp_name_size-i)*sizeof(char));
			for(j=file_out;i<=temp_name_size;i++,j++){
				temp_destination[j]=temp_name[i];
			}
			//End process of filtering the name
			//Start Update Screen
			//Atualiza porcentagem
			j=(percent_total*100)/total_files;
			for(z=0,k=0;z<=j ;z+=3.3,k+=2){
				percent_box[k]=(char)219;
			}
			i=j/100;// 345 -> 3
			l=j/10;// 345 -> 34
			if(l>9)l=l-i*10;// 34 -> 4
			k=j-(i*100+l*10);
			if(j>99)number_percent[0]=i+48;
			if(j>9)number_percent[1]=l+48;
			number_percent[2]=k+48;
			insert_string_on_display("(ENTER=CANCELAR)",pos_ver+6,pos_hor+47,DISPLAY_WINDOW2);
			insert_string_on_display(percent_box,pos_ver+5,pos_hor+2,DISPLAY_WINDOW2);
			insert_string_on_display(number_percent,pos_ver+4,pos_hor+61,DISPLAY_WINDOW2);
			print_display(DISPLAY_WINDOW2);
			//END UPDATE SCREEN
			//Start file copy paste process
			if(!check_adress_basic_test(temp_name)){
				printf("ERROR %s use a wrong adress... skiping copy",temp_name);
				out_loop=true;
			}
			else{
				in=open_file(temp_name,"rb");
				if(!in){
					printf("ERROR %s file found...",temp_name);
					out_loop=true;
				}
				else if(check_if_file_exist(temp_destination)){
					printf("replace file? (Y/N)");
					i=Keyboard_keytouched();
					if(i==_key)Keyboard_keytouched();
					if(i=='n' || i=='N') out_loop=true;
					else{
						out=open_file(temp_destination,"wb");
					}
				}
				i=0;
			}
			if(out_loop==false){
				while(!feof(in)){
					buff=fgetc(in);
					fputc(buff,out);
				}
			}
			//End file copy paste process
			if(out_loop==false){
				i;
				fclose(in);
				fclose(out);
			}free(temp_name);
			percent_total++;
			out_loop=false;
		}
		free(temp_destination);
	}else{
			print_popup("Error, no file found. Aborting...");
	}
}
