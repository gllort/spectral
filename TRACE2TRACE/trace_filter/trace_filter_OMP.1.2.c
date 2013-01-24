#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

#define MAX_HEADER_SIZE 131072
#define MAX_THREADS 16
#define MAX_SELECTED_TASKS 30

/* Buffer for reading trace records */
char line[4096];

/* Execution parameters */
int show_states = 0, show_comms = 0, show_events = 0, filter_tasks = 0;
int all_types = 0;
unsigned long long min_state_time = 0, max_state_time = 0;
int min_comm_size = 0;
unsigned long long num_records = 0;

/* Trace in and trace out */
FILE *infile[8], *outfile[8];

/* Structs */
struct types
{
	unsigned long long min_type;
	unsigned long long max_type;
	int ranged_type;
};

struct types allowed_types[20];
int next_allowed_type = 0;

/* struct for cutting only selected tasks */
struct selected_tasks
{

	int min_task_id;
	int max_task_id;
	int range;
};

struct selected_tasks wanted_tasks[MAX_SELECTED_TASKS];

/* Parameter for showing the percentage */
unsigned long long total_trace_size, total_subtrace_size;
#if defined(trace_filter_lib)
	double last_showed_trace_filter = 0;
#else
	double last_showed = 0;
#endif
unsigned long total_iters = 0;

/* Function for parsing program parameters */
#if defined(trace_filter_lib)
void read_args_trace_filter(int argc, char *argv[])
{
#else
void read_args(int argc, char *argv[])
{
#endif

	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	int i,j;
	char *word, *types_allowed, *word2;

	if(argc < 4)
	{
		printf("\nApplication for filtering states, events or communications in Paraver traces\n\n");
		printf("Usage:  %s < trace_in > < trace_out > < options >\n\n",argv[0]);
		printf("Options:\n");
		printf("  --help                           Display this info\n");
		printf("  -show_states                     Put info of the running states in the trace\n");
		printf("  -show_states_min_time:<time>     Put info of the running states greater than\n");
		printf("                                   <time> ns\n");
		printf("  -show_states_max_time:<time>     Put info of the running states smaller than\n");
		printf("                                   <time> ns\n");
		printf("  -show_comms[:min_comm_size]      Put the communications in the trace_out\n");
		printf("  -show_events[:type1[-type2],...] Put only these call types or a range of calls\n");
		printf("                                   in the trace out.\n");
		printf("  -tasks id_1[,id_2...]            Put only info of the tasks specified in the\n");
		printf("                                   list. The id could be a task number or\n");
		printf("                                   a range of tasks\n");
		printf("\n  Flag -show_events without any type, put all event types into the trace\n\n");
		printf("  You can set environment var. OMP_NUM_THREADS in order to %s uses OMP threads\n\n", argv[0]);
		printf("\n  Examples:\n\t%s trace1.prv trace2.prv -show_states_min_time:200000 -show_types:2001,50000001-50000003 -tasks 5,10-20,31\n\n",argv[0]);
		exit(0);
	}

	i = 3;
	while(i < argc)
	{

		if(strstr(argv[i],"-show_states_min_time"))
		{
			show_states = 1;

			word = strtok(argv[i],":");

			if((word = strtok(NULL,":"))!=NULL)
				min_state_time = atoll(word);

			i++; continue;
		}

		if(strstr(argv[i],"-show_states_max_time"))
		{
			show_states = 1;

			word = strtok(argv[i],":");

			if((word = strtok(NULL,":"))!=NULL)
				max_state_time = atoll(word);

			i++; continue;
		}

		if(strstr(argv[i],"-show_states"))
		{
			show_states = 1;

			i++; continue;
		}

		if(strstr(argv[i],"-show_comms"))
		{
			show_comms = 1;

			word = strtok(argv[i],":");

			if((word = strtok(NULL,":"))!=NULL)
				min_comm_size = atoll(word);

			i++; continue;
		}

		if(strstr(argv[i], "-tasks"))
		{
			filter_tasks = 1;
			int j = 0;

			word = strtok(argv[i+1], ",");
			do
			{
				if((word2 = strchr(word, '-')) != NULL)
				{
					*word2 = '\0';
					wanted_tasks[j].min_task_id = atoll(word);
					wanted_tasks[j].max_task_id = atoll(++word2);
					wanted_tasks[j].range = 1;
				}
				else
				{
					wanted_tasks[j].min_task_id = atoll(word);
					wanted_tasks[j].range = 0;
				}

				j++;
			}
			while((word = strtok(NULL, ",")) != NULL);

			i+=2; continue;

		}

		if(strstr(argv[i],"-show_events"))
		{
			show_events = 1;

			word = strtok(argv[i],":");

			if((types_allowed = strtok(NULL,":"))!=NULL)
			{
				word = strtok(types_allowed, ",");
				do
				{
					if((word2 = strchr(word,'-'))!=NULL)
					{
						*word2='\0';
						allowed_types[next_allowed_type].min_type = atoll(word);
						allowed_types[next_allowed_type].max_type = atoll(++word2);
						allowed_types[next_allowed_type].ranged_type = 1;

					}
					else
					{
						allowed_types[next_allowed_type].min_type = atoll(word);
						allowed_types[next_allowed_type].ranged_type = 0;
					}
					next_allowed_type++;
				}
				while((word = strtok(NULL,","))!=NULL);
			}
			else all_types = 1;

			i++;
		}

	}

	if(max_state_time != 0 && min_state_time > max_state_time)
	{
		printf("ERROR: wrong time interval for filtering state bursts!!!\n");
		exit(1);
	}
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif
}

/* For processing the Paraver header */
unsigned long long proces_header(char *header, FILE *infile, FILE *outfile)
{
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	int num_comms;
	char *word;
	unsigned long long readedBytes = 0;

	/* Obtaining the total trace time */
	readedBytes = strlen(header);
	word = strtok(header,":");
	word = strtok(NULL, ":");
	word = strtok(NULL, ":");

	word = strtok(NULL,"\n");
	word = strrchr(word,',');
	if(word == NULL)
	{
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_user_function(0);
		#endif
		return;
	}

	/* Obtaining th number of communicators */
	strcpy(header, word+1);
	if(strchr(header,')')!=NULL)
	{
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_user_function(0);
		#endif
		return;
	}
	num_comms = atoi(header);

	while(num_comms > 0)
	{
		fgets(header, MAX_HEADER_SIZE, infile);
		readedBytes += strlen(header);
		fprintf(outfile,"%s",header);
		num_comms--;
	}
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif
	return readedBytes;
}

void ini_progress_bar(unsigned long long size_per_thread)
{
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	
	total_subtrace_size = size_per_thread;

	/* Depen mida traça mostrem percentatge amb un interval diferent de temps */
	if(total_subtrace_size < 500000000) total_iters = 500000;
	else total_iters = 5000000;
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif

}

void show_progress_bar(unsigned long long current_readed_size)
{

	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	double current_showed;

	current_showed = ceil(((double)current_readed_size/(double)total_subtrace_size)*100);
	#if defined(trace_filter_lib)
		if(last_showed_trace_filter != current_showed)
		{
			printf("...%2.0f%%", current_showed);
			last_showed_trace_filter = current_showed;
		}
	#else
		if(last_showed != current_showed)
		{
			printf("...%2.0f%%", current_showed);
			last_showed = current_showed;
		}
	#endif
	fflush(stdout);
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif

}

int align_threads(FILE *infile, unsigned long long offset)
{

	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	char c;
	int readedBytes = 0;

	fseek(infile, offset, SEEK_SET);

	while(( c = (char)fgetc(infile)) != EOF)
	{
		readedBytes++;
		if( c == '\n') break;
	}
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif
	return readedBytes;
}

#if defined(trace_filter_lib)
void copy_file_trace_filter(char *in, char *out)
{
#else
void copy_file(char *in, char *out)
{
#endif
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	FILE *fileIn, *fileOut;
	char line[2048];

	if((fileIn = fopen(in,"r")) == NULL) 
	{
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_user_function(0);
		#endif
		return;
	}
	fileOut = fopen(out,"w");

	while(fgets(line,sizeof(line),fileIn)!=NULL)
		fputs(line, fileOut);

	fclose(fileIn); fclose(fileOut);
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif

}

void cat_file(char *in, char *out)
{
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	
	FILE *fileIn, *fileOut;
	char line[2048];

	if((fileIn = fopen(in,"r")) == NULL)
	{	
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_user_function(0);
		#endif
		return;
	}
	fileOut = fopen(out,"a");

	while(fgets(line,sizeof(line),fileIn)!=NULL)
		fputs(line, fileOut);

	fclose(fileIn); fclose(fileOut);
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif

}

/* Function for filtering tasks in cut */
#if defined(trace_filter_lib)
int is_selected_task_trace_filter(int task_id)
{
#else
int is_selected_task(int task_id)
{
#endif
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	int i;

	for(i = 0; i < MAX_SELECTED_TASKS; i++)
	{
		if(wanted_tasks[i].min_task_id == 0) break;

		if(wanted_tasks[i].range)
		{
			if(task_id >= wanted_tasks[i].min_task_id && task_id <= wanted_tasks[i].max_task_id) 
			{
				#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
					Extrae_user_function(0);
				#endif
				return 1;
			}
		}
		else
		if(task_id == wanted_tasks[i].min_task_id) 
		{
			#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
				Extrae_user_function(0);
			#endif
			return 1;
		}
	}
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif
	return 0;
}

#if defined(trace_filter_lib)
int trace_filter_main(int argc, char *argv[])
{
#else
int main(int argc, char *argv[])
{
#endif
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(1);
	#endif
	int i, num_char, end_line, print_record, allowed_type, task, task_2, state, size;
	unsigned long long time_1, time_2, type, value;
	char *word[MAX_THREADS], *saveptr[MAX_THREADS], event_record[2048], buffer[1024], *trace_header, *trace_name;
	unsigned long num_iters = 0;

	char trace_files[MAX_THREADS][512];

	int tid = 0, nThreads = 1;
	unsigned long long offset, size_per_thread, size_processed;
	struct stat file_info;

	#if defined(trace_filter_lib)
		show_states = 0; show_comms = 0; show_events = 0; filter_tasks = 0;
		all_types = 0; min_state_time = 0; max_state_time = 0;
		min_comm_size = 0; num_records = 0;
		next_allowed_type = 0;last_showed_trace_filter = 0;total_iters = 0;
	#endif

	/* Reading of the program arguments */
	#if defined(trace_filter_lib)
		read_args_trace_filter(argc,argv);
	#else
		read_args(argc,argv);
	#endif

	trace_name = strdup(argv[1]);

	if(stat(trace_name, &file_info)<0)
	{
		perror("Error calling stat64");
		exit(1);
	}
	total_trace_size = file_info.st_size;

	/* Generating subtraces names */
	strcpy(trace_files[0], argv[2]);
	#if (defined(OMP_FLAG) || defined(OMPSS_FLAG))
		nThreads = omp_get_max_threads();
		for(i = 1; i < nThreads; i++)
			strcpy(trace_files[i], "/tmp/tmp_traceXXXXXX");
	#endif

	/* Copy of the .pcf */
	char *pcfIn, *pcfOut;

	pcfIn = strdup(trace_name);
	pcfOut = strdup(argv[2]);
	pcfIn[strlen(pcfIn)-4] = '\0';
	pcfOut[strlen(pcfOut)-4] = '\0';
	sprintf(pcfIn, "%s.pcf", pcfIn);
	sprintf(pcfOut, "%s.pcf", pcfOut);
	#if defined(trace_filter_lib)
		copy_file_trace_filter(pcfIn, pcfOut);
	#else
		copy_file(pcfIn, pcfOut);
	#endif
	free(pcfIn);

	/* Copiem el .row */
	char *rowIn, *rowOut;

	rowIn = strdup(trace_name);
	rowOut = strdup(argv[2]);
	rowIn[strlen(rowIn)-4] = '\0';
	rowOut[strlen(rowOut)-4] = '\0';
	sprintf(rowIn, "%s.row", rowIn);
	sprintf(rowOut, "%s.row", rowOut);
	#if defined(trace_filter_lib)
		copy_file_trace_filter(rowIn, rowOut);
	#else
		copy_file(rowIn, rowOut);
	#endif
	free(rowIn);

	printf("\nFiltering trace...");
	fflush(stdout);

	#if defined(OMP_FLAG)
	#pragma omp parallel reduction(+ : num_records) default(shared) private(tid, task, task_2, nThreads, offset, size_per_thread, size_processed, line, time_1, time_2, type, value, state, size, i, num_char, end_line, print_record, allowed_type, event_record, buffer)

	{
	#endif
	
	#if defined(OMPSS_FLAG)
	int iGen;
	for(iGen=0;iGen<omp_get_num_threads();iGen++)//para generar los tasks
	{
		
	#pragma omp task concurrent(num_records) default(shared) private(tid, task, task_2, nThreads, offset, size_per_thread, size_processed, line, time_1, time_2, type, value, state, size, i, num_char, end_line, print_record, allowed_type, event_record, buffer)
	{
	#endif
		/* Obtain thread identifier and some info */
	#if (defined(OMP_FLAG) || defined(OMPSS_FLAG))
		tid = omp_get_thread_num();
		nThreads = omp_get_num_threads();
		
	#endif

		size_per_thread = total_trace_size / nThreads;
		offset = size_per_thread * tid;
		size_processed = 0;

		/* Open files */
		if((infile[tid] = fopen(trace_name,"r")) == NULL)
		{
			printf("Error Opening File %s\n",trace_name);
			exit(1);
		}

		if(tid != 0)
			mkstemp(trace_files[tid]);

		if((outfile[tid] = fopen(trace_files[tid],"w")) == NULL)
		{
			printf("Error Opening File %s\n", trace_files[tid]);
			exit(1);
		}

		if(tid == 0)
		{

			ini_progress_bar(size_per_thread);

			/* Process header, only the master */
			trace_header = (char *)malloc(sizeof(char)*MAX_HEADER_SIZE);
			fgets(trace_header, MAX_HEADER_SIZE, infile[0]);
			fprintf(outfile[0],"%s",trace_header);
			size_processed += proces_header(trace_header, infile[0], outfile[0]);
			free(trace_header);
		}
		else
		{

			size_processed = align_threads(infile[tid], offset);
		}
		
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_event (1001, 1);
		#endif
		/* Processing the trace records */
		while(fgets(line,sizeof(line),infile[tid])!=NULL)
		{
			size_processed += strlen(line);
			/*#ifndef trace_filter_lib
				printf("HE AGAFAT %s\n", line); fflush(stdout);
			#endif*/
			if(tid == 0)
			{

				if(num_iters == total_iters)
				{
					show_progress_bar(size_processed);
					num_iters = 0;
				}
				else num_iters++;

			}

			/* 1: state; 2: event; 3: comm; 4: global comm */
			switch(line[0])
			{

				case '1':

					/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
						Extrae_event (1001, 2);
					#endif*/
					if(!show_states) break;

					sscanf(line,"%*d:%*d:%*d:%d:%*d:%lld:%lld:%d\n",&task, &time_1, &time_2, &state);
					#if defined(trace_filter_lib)
						if(filter_tasks && !is_selected_task_trace_filter(task)) break;
					#else
						if(filter_tasks && !is_selected_task(task)) break;
					#endif
					if(state != 1) break;

					if(min_state_time > 0 && (min_state_time > (time_2 - time_1))) break;

					if(max_state_time > 0 && (max_state_time < time_2 - time_1)) break;
					#if defined(OMPSS_FLAG)
						#pragma omp atomic
					#endif
					num_records++;
					fputs(line,outfile[tid]);

					break;

				case '2':
					/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
						Extrae_event (1001, 3);
					#endif*/
					if(!show_events) break;

					strncpy(buffer, line, sizeof(buffer));
					sscanf(buffer, "%*d:%*d:%*d:%d:%*s\n", &task);
					#if defined(trace_filter_lib)
						if(filter_tasks && !is_selected_task_trace_filter(task)) break;
					#else
						if(filter_tasks && !is_selected_task(task)) break;
					#endif

					if(all_types)
					{
						#if defined(OMPSS_FLAG)
							#pragma omp atomic
						#endif
						num_records++;
						fputs(line, outfile[tid]);
						break;
					}

					i = 0; num_char = 0;
					while(1)
					{
						if(line[i] == ':')
						{
							num_char++;
							if(num_char == 6)
							{
								line[i] = '\0';
								break;
							}
						}
						i++;
					}

					sprintf(event_record,"%s",line);

					/* Event type and values */
					end_line = 0; print_record = 0;
					word[tid]=strtok_r(&line[i+1],":", &saveptr[tid]);
					type = atoll(word[tid]);
					word[tid]=strtok_r(NULL,":", &saveptr[tid]);
					value=atoll(word[tid]);

					allowed_type = 0;
					for(i=0; i<next_allowed_type; i++)
					{
						if(allowed_types[i].ranged_type)
						{
							if(type <= allowed_types[i].max_type && type >= allowed_types[i].min_type)
							{
								allowed_type = 1;
								break;
							}
						}
						else
						if(type == allowed_types[i].min_type)
						{
							allowed_type = 1;
							break;
						}
					}

					if(allowed_type)
					{
						print_record = 1;
						sprintf(event_record,"%s:%lld:%lld",event_record,type,value);
					}

					end_line = 0;
					while(!end_line)
					{

						if((word[tid]=strtok_r(NULL,":", &saveptr[tid]))!=NULL)
						{
							type = atoll(word[tid]);
							word[tid]=strtok_r(NULL,":", &saveptr[tid]);
							value=atoll(word[tid]);

							allowed_type = 0;
							for(i=0; i<next_allowed_type; i++)
							{
								if(allowed_types[i].ranged_type)
								{
									if(type <= allowed_types[i].max_type && type >= allowed_types[i].min_type)
									{
										allowed_type = 1;
										break;
									}
								}
								else
								if(type == allowed_types[i].min_type)
								{
									allowed_type = 1;
									break;
								}
							}

							if(allowed_type)
							{
								print_record = 1;
								sprintf(event_record,"%s:%lld:%lld",event_record,type,value);
							}
						}
						else
						{
							end_line = 1;
							sprintf(event_record,"%s\n",event_record);
						}
					}

					if(print_record)
					{
						#if defined(OMPSS_FLAG)
							#pragma omp atomic
						#endif
						num_records++;
						fputs(event_record,outfile[tid]);
					}
					break;

				case '3':
					/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
						Extrae_event (1001, 4);
					#endif*/
					if(!show_comms) break;

					sscanf(line,"%*d:%*d:%*d:%d:%*d:%*lld:%*lld:%*d:%*d:%d:%*d:%*lld:%*lld:%d:%*d\n", &task, &task_2, &size);
					#if defined(trace_filter_lib)
						if(filter_tasks && (!is_selected_task_trace_filter(task)||!is_selected_task_trace_filter(task))) break;
					#else
						if(filter_tasks && (!is_selected_task(task)||!is_selected_task(task))) break;
					#endif
					if(min_comm_size > 0)
					{
						if(size < min_comm_size) break;
					}
					#if defined(OMPSS_FLAG)
						#pragma omp atomic
					#endif
					num_records++;
					fputs(line,outfile[tid]);
					break;

				case '4':
					/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
						Extrae_event (1001, 5);
					#endif*/
					fputs(line,outfile[tid]);
					break;

				default: 
					/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
						Extrae_event (1001, 6);
					#endif*/
					break;
			}
			/*#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
				Extrae_event (1001, 0);
			#endif*/
			if(size_processed >= size_per_thread) break;

		}
		#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
			Extrae_event (1001, 0);
		#endif
		/* Close the files */
		fclose(infile[tid]); fclose(outfile[tid]);

	#if (defined(OMP_FLAG) || defined(OMPSS_FLAG))
	}//del pragma omp ...
	#endif
	#if defined(OMPSS_FLAG)
	}//for para generar las task
		#pragma omp taskwait		
	#endif
	
	printf("...Done\n\n");

	#if (defined(OMP_FLAG) || defined(OMPSS_FLAG))
		nThreads = omp_get_max_threads();
	#endif
		
	if(nThreads > 1)
	{
		printf("Merging subtraces generated...");

		for(i = 1; i < nThreads; i++)
		{

			cat_file(trace_files[i], trace_files[0]);

			unlink(trace_files[i]);
		}

		printf("...Done\n\n");
	}

	if(num_records == 0)
	{
		printf("WARNING!!! Trace not generated. The trace doesn't have any records\n\n");
		unlink(trace_files[0]);
		unlink(pcfOut); free(pcfOut);
		unlink(rowOut); free(rowOut);
	}
	
	#if (defined(TRACE_MODE) && !defined(OMPSS_FLAG))
		Extrae_user_function(0);
	#endif
}
