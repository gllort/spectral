#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <unistd.h>
#include <zlib.h>
#include "config.h"

#define MAX_TRACE_HEADER 131072
#define MAX_APPL 2
#define MAX_TASK 15000
#define MAX_THREAD 10
#define MAX_SELECTED_TASKS 30

/* Buffer for reading trace records */
static char line[4096];

/* Execution parameters */
unsigned int min_perc = 0, max_perc =
  100, by_time, old_times, max_size, is_zip;
unsigned int cut_tasks, break_states, keep_events = 0;
unsigned long long time_min, time_max, total_time, first_record_time,
  last_record_time;
unsigned long long current_size, total_size, trace_time;
int num_tasks, usefull_tasks, init_task_counter;
unsigned int remFirstStates, remLastStates, first_time_caught;

/* Trace in and trace out */
FILE *infile, *outfile;
gzFile gzInfile;

/* Struct for the case of MAX_TRACE_SIZE */
struct thread_info
{
  int appl;
  int task;
  int thread;
  unsigned long long last_time;
  char finished;
  unsigned long long event_queue[20];
  int first;
  int last;
  struct thread_info *next;
  struct thread_info *previous;
};

struct thread_info *tasks[MAX_APPL][MAX_TASK][MAX_THREAD];
struct thread_info *first;
struct thread_info *last;

/* Parameters for showing percentage */
unsigned long long total_trace_size, current_readed_size;
static unsigned long total_cutter_iters = 0;

/* Vars for saving the HC that will appear on the trace */
unsigned long long counters[50];
int last_counter;

/* struct for cutting only selected tasks */
struct selected_tasks
{

  int min_task_id;
  int max_task_id;
  int range;
};

struct selected_tasks wanted_tasks[MAX_SELECTED_TASKS];

double cutter_last_showed = 0;

/* Function for parsing program parameters */

void
cutter_read_args (int argc, char *argv[])
{

  int i;
  char *envptr, *buffer, *word;

  if (argc < 5)
    {
      printf
	("\nApplication for cutting Paraver traces by time or percentage\n\n");
      printf ("Usage:  %s < trace_in > < trace_out > < options >\n\n",
	      argv[0]);
      printf ("Options:\n");
      printf ("  --help\t\t\t\tDisplay this info\n");
      printf ("  -t  <time_min> <time_max>\t\tCut traces by time in ns\n");
      printf
	("  -p  <percent_min> <percent_max>\tCut traces by percentage of time\n");
      printf
	("  -original_time\t\t\tKeep times of the original trace.\n\t\t\t\t\tIf this flag is not specified, the\n\t\t\t\t\tnew cutted trace will start at time 0\n");
      printf
	("  -tasks <id_1[,id_2...]]>\t\tPut info only for the identifiers\n\t\t\t\t\tspecified in the list where every id\n\t\t\t\t\tcould be a task_id or a range of tasks\n\t\t\t\t\t[task_id_1-task_id_2]\n");
      printf
	("  -not_break_states\t\t\tWhen cutting without original time, \n\t\t\t\t\tdon't split states due to the cut\n");
      printf
	("  -keep_boundary_events\t\t\tKeep the events outside the cutting zone\n\t\t\t\t\tof the bursts delimiting the cut\n");
      printf
	("  -remove_first_state\t\t\tOmmit states which don't fit entirely \n\t\t\t\t\tin the beginnig of the cut\n");
      printf
	("  -remove_last_state\t\t\tOmmit states which don't fit entirely \n\t\t\t\t\tin the end of the cut\n");
      printf
	("\nExamples:\n\t%s trace1.prv trace2.prv -p 10 35 -tasks 1-20,35,40-43\n\n",
	 argv[0]);
      printf
	("\nUsers can set the environment variable CUTTER_LOGFILE pointing to a file that will serve as a log of the cutted traces\n");
      printf
	("\nUsers can set the environment variable MAX_TRACE_SIZE with the value of the maximum size for the output trace\n");
      exit (1);
    }

  i = 3;
  while (i < argc)
    {

      if (!strcmp (argv[i], "-t"))
	{
	  by_time = 1;
	  time_min = atoll (argv[i + 1]);
	  time_max = atoll (argv[i + 2]);

	  if (time_max <= time_min)
	    {
	      printf ("ERROR: The time interval is incorrect\n");
	      exit (1);
	    }

	  total_time = time_max - time_min;
	  i += 3;
	  continue;
	}

      if (!strcmp (argv[i], "-p"))
	{
	  min_perc = atoi (argv[i + 1]);
	  max_perc = atoi (argv[i + 2]);

	  if (max_perc <= min_perc || max_perc > 100 || min_perc < 0)
	    {
	      printf ("ERROR: The percentages are incorrect\n");
	      exit (1);
	    }

	  i += 3;
	  continue;
	}

      if (!strcmp (argv[i], "-original_time"))
	{
	  old_times = 1;
	  i++;
	  continue;
	}

      if (!strcmp (argv[i], "-tasks"))
	{
	  int j = 0;
	  cut_tasks = 1;
	  word = strtok (argv[i + 1], ",");
	  do
	    {
	      if ((buffer = strchr (word, '-')) != NULL)
		{
		  *buffer = '\0';
		  wanted_tasks[j].min_task_id = atoll (word);
		  wanted_tasks[j].max_task_id = atoll (++buffer);
		  wanted_tasks[j].range = 1;
		}
	      else
		{
		  wanted_tasks[j].min_task_id = atoll (word);
		  wanted_tasks[j].range = 0;
		}

	      j++;
	    }
	  while ((word = strtok (NULL, ",")) != NULL);

	  i += 2;
	  continue;
	}

      if (!strcmp (argv[i], "-not_break_states"))
	{
	  break_states = 0;
	  i++;
	  continue;
	}

      if (!strcmp (argv[i], "-remove_first_state"))
	{
	  remFirstStates = 1;
	  i++;
	  continue;
	}

      if (!strcmp (argv[i], "-remove_last_state"))
	{
	  remLastStates = 1;
	  i++;
	  continue;
	}

      if (!strcmp (argv[i], "-keep_boundary_events"))
	{
	  keep_events = 1;
	  i++;
	  continue;
	}

    }

  envptr = getenv ("MAX_TRACE_SIZE");
  if (envptr != NULL)
    max_size = atoi (envptr) * 1000000;

}

/* For processing the Paraver header */
void
proces_cutter_header (char *header, char *trace_in_name, char *trace_out_name)
{

  int num_comms;
  char *word;

  word = strtok (header, ":");
  current_size += fprintf (outfile, "%s:", word);

  word = strtok (NULL, ":");
  current_size += fprintf (outfile, "%s:", word);

  /* Obtaining the trace total time */
  word = strtok (NULL, ":");

  if (strstr (word, "_ns"))
    {
      word[strlen (word) - 3] = '\0';
      trace_time = atoll (word);

      if (!by_time)
	{
	  trace_time = atoll (word);
	  time_min = ((double) (trace_time / 100)) * min_perc;
	  time_max = ((double) (trace_time / 100)) * max_perc;
	  total_time = time_max - time_min;
	}

      if (!old_times)
	current_size += fprintf (outfile, "%lld_ns:", total_time);
      else
	current_size += fprintf (outfile, "%s_ns:", word);
    }
  else
    {
      trace_time = atoll (word);
      if (!by_time)
	{
	  trace_time = atoll (word);
	  time_min = ((double) (trace_time / 100)) * min_perc;
	  time_max = ((double) (trace_time / 100)) * max_perc;
	  total_time = time_max - time_min;
	}

      if (!old_times)
	current_size += fprintf (outfile, "%lld:", total_time);
      else
	current_size += fprintf (outfile, "%s:", word);

    }

  word = strtok (NULL, "\n");
  current_size += fprintf (outfile, "%s\n", word);

  if ((word = strrchr (word, ',')) == NULL)
    return;

  /* Obtaining th number of communicators */
  strcpy (header, word + 1);
  if (strchr (header, ')') != NULL)
    return;
  num_comms = atoi (header);

  while (num_comms > 0)
    {
      if (!is_zip)
	fgets (header, MAX_TRACE_HEADER, infile);
      else
	gzgets (gzInfile, header, MAX_TRACE_HEADER);
      current_size += fprintf (outfile, "%s", header);
      num_comms--;
    }

  /* Writting in the header the offset of the cut regard original trace */

  /* Reading first if we have old offsets into the trace */
  if (!is_zip)
    fgets (header, MAX_TRACE_HEADER, infile);
  else
    gzgets (gzInfile, header, MAX_TRACE_HEADER);

  while (header[0] == '#')
    {
      current_size += fprintf (outfile, "%s", header);

      if (!is_zip)
	fgets (header, MAX_TRACE_HEADER, infile);
      else
	gzgets (gzInfile, header, MAX_TRACE_HEADER);
    }

  fseek (infile, -(strlen (header)), SEEK_CUR);

  /* Writting of the current cut offset */
  if (trace_in_name != '\0')
    current_size +=
      fprintf (outfile, "# %s: Offset %lld from %s\n", trace_out_name,
	       time_min, trace_in_name);

}


void
adjust_to_final_time ()
{

  unsigned long long next_time = 0;
  struct thread_info *p, *q;

  while (num_tasks > 0)
    {

      next_time = first->last_time;

      q = first;
      for (p = first->next; p != NULL; p = p->next)
	if (p->last_time < next_time)
	  {
	    next_time = p->last_time;
	    q = p;
	  }

      if (old_times)
	fprintf (outfile, "1:0:%d:%d:%d:%lld:%lld:14\n", q->appl, q->task,
		 q->thread, next_time, trace_time);

      if (q->first != (q->last + 1) % 20)
	{

	  while (q->first != q->last)
	    {
	      fprintf (outfile, "2:%d:%d:%d:%d:%lld:%lld:0\n", q->task,
		       q->appl, q->task, q->thread, next_time,
		       q->event_queue[q->first]);
	      q->first = (q->first + 1) % 20;
	    }

	  fprintf (outfile, "2:%d:%d:%d:%d:%lld:%lld:0\n", q->task, q->appl,
		   q->task, q->thread, next_time, q->event_queue[q->first]);
	}

      num_tasks--;
      next_time = 0;
      if (num_tasks > 0)
	{
	  if (q == first)
	    {
	      first = q->next;
	      first->previous = NULL;
	    }
	  else
	    {
	      if (q == last)
		{
		  last = q->previous;
		  last->next = NULL;
		}
	      else
		{
		  (q->next)->previous = q->previous;
		  (q->previous)->next = q->next;
		}
	    }
	}
      free (q);
    }

}


void
ini_cutter_progress_bar (char *file_name)
{

  struct stat file_info;

  if (stat (file_name, &file_info) < 0)
    {
      perror ("Error calling stat64");
      exit (1);
    }
  total_trace_size = file_info.st_size;

  /* Depen mida traça mostrem percentatge amb un interval diferent de temps */
  if (total_trace_size < 500000000)
    total_cutter_iters = 500000;
  else
    total_cutter_iters = 5000000;

  current_readed_size = 0;

}


void
show_cutter_progress_bar ()
{

  double current_showed, i, j;

#if defined(OS_FREEBSD)
  if (!is_zip)
    current_readed_size = (unsigned long long) ftello (infile);
  else
    current_readed_size = (unsigned long) gztell (gzInfile);
#else
  if (!is_zip)
    current_readed_size = (unsigned long long) ftello64 (infile);
  else
    current_readed_size = (unsigned long) gztell (gzInfile);
#endif

  i = (double) (current_readed_size);
  j = (double) (total_trace_size);

  current_showed = i / j;

  current_showed = current_showed * 100;
  if (cutter_last_showed != current_showed)
    {
      printf ("...%2.0f%%", current_showed);
      cutter_last_showed = current_showed;
    }

  fflush (stdout);

}


void
update_queue (int appl, int task, int thread, unsigned long long type,
	      unsigned long long value)
{

  int i;
  struct thread_info *p;

  if (tasks[appl][task][thread] == NULL)
    {
      if ((p =
	   (struct thread_info *) malloc (sizeof (struct thread_info))) ==
	  NULL)
	{
	  perror ("No more memory!!!\n");
	  exit (1);
	}
      if (first == NULL)
	{
	  first = p;
	  p->previous = NULL;
	}

      p->next = NULL;
      if (last == NULL)
	last = p;
      else
	{
	  p->previous = last;
	  last->next = p;
	  last = p;
	}

      p->appl = appl + 1;
      p->task = task + 1;
      p->thread = thread + 1;

      tasks[appl][task][thread] = p;
      tasks[appl][task][thread]->last_time = 0;
      tasks[appl][task][thread]->finished = 0;
      tasks[appl][task][thread]->first = 0;
      tasks[appl][task][thread]->last = -1;

      num_tasks++;
      usefull_tasks++;
      init_task_counter = 1;
    }

  if (value > 0)
    {

      tasks[appl][task][thread]->last =
	(tasks[appl][task][thread]->last + 1) % 20;
      tasks[appl][task][thread]->event_queue[tasks[appl][task][thread]->
					     last] = type;

    }
  else
    {

      if (tasks[appl][task][thread]->first ==
	  (tasks[appl][task][thread]->last + 1) % 20)
	return;

      for (i = tasks[appl][task][thread]->first;
	   i != tasks[appl][task][thread]->last; i = (i + 1) % 20)
	if (tasks[appl][task][thread]->event_queue[i] == type)
	  break;

      tasks[appl][task][thread]->first = (i + 1) % 20;
    }

}


void
load_counters_of_pcf (char *trace_name)
{

  char *pcf_name, *c;
  FILE *pcf;
  char *id;

  pcf_name = strdup (trace_name);
  c = strrchr (pcf_name, '.');
  sprintf (c, ".pcf");

  last_counter = 0;
  if ((pcf = fopen (pcf_name, "r")) == NULL)
    return;

  while (fgets (line, sizeof (line), pcf) != NULL)
    {

      if (strstr (line, " 42000") != NULL || strstr (line, " 42001") != NULL)
	{
	  id = strtok (line, " ");
	  id = strtok (NULL, " ");

	  counters[last_counter] = atoll (id);
	  last_counter++;

	  if (last_counter == 50)
	    {
	      printf ("NO more memory for loading counters of .pcf\n");
	      return;
	    }
	}
    }
}


void
shift_trace_to_zero (char *nameIn, char *nameOut)
{

  unsigned long long timeOffset = 0, time_1, time_2, time_3, time_4;
  int cpu, appl, task, thread, state, cpu_2, appl_2, task_2,
    thread_2;
  char *trace_header;

#if defined(OS_FREEBSD)
  if ((infile = fopen (nameIn, "r")) == NULL)
    {
      perror ("ERROR");
      printf ("Cutter: Error Opening File %s\n", nameIn);
      exit (1);
    }
#else
  if ((infile = fopen64 (nameIn, "r")) == NULL)
    {
      perror ("ERROR");
      printf ("Cutter: Error Opening File %s\n", nameIn);
      exit (1);
    }
#endif

#if defined(OS_FREEBSD)
  if ((outfile = fopen (nameOut, "w")) == NULL)
    {
      perror ("ERROR");
      printf ("Cutter: Error Opening File %s\n", nameOut);
      exit (1);
    }
#else
  if ((outfile = fopen64 (nameOut, "w")) == NULL)
    {
      perror ("ERROR");
      printf ("Cutter: Error Opening File %s\n", nameOut);
      exit (1);
    }
#endif

  /* Process header */
  total_time = last_record_time - first_record_time;
  trace_header = (char *) malloc (sizeof (char) * MAX_TRACE_HEADER);
  fgets (trace_header, MAX_TRACE_HEADER, infile);

  proces_cutter_header (trace_header, '\0', '\0');

  fgets (trace_header, MAX_TRACE_HEADER, infile);
  sscanf (trace_header, "%*d:%*d:%*d:%*d:%*d:%lld:", &timeOffset);

  int end_read = 0;
  while (!end_read)
    {
      switch (trace_header[0])
	{
	case '1':
	  sscanf (trace_header, "%*d:%d:%d:%d:%d:%lld:%lld:%d\n", &cpu, &appl,
		  &task, &thread, &time_1, &time_2, &state);

	  time_1 = time_1 - timeOffset;
	  time_2 = time_2 - timeOffset;

	  fprintf (outfile, "1:%d:%d:%d:%d:%lld:%lld:%d\n", cpu, appl, task,
		   thread, time_1, time_2, state);

	  break;

	case '2':
	  sscanf (trace_header, "%*d:%d:%d:%d:%d:%lld:%s", &cpu, &appl, &task,
		  &thread, &time_1, line);

	  time_1 = time_1 - timeOffset;

	  fprintf (outfile, "2:%d:%d:%d:%d:%lld:%s\n", cpu, appl, task,
		   thread, time_1, line);

	  break;

	case '3':
	  sscanf (trace_header,
		  "%*d:%d:%d:%d:%d:%lld:%lld:%d:%d:%d:%d:%lld:%lld:%s", &cpu,
		  &appl, &task, &thread, &time_1, &time_2, &cpu_2, &appl_2,
		  &task_2, &thread_2, &time_3, &time_4, line);

	  time_1 = time_1 - timeOffset;
	  time_2 = time_2 - timeOffset;
	  time_3 = time_3 - timeOffset;
	  time_4 = time_4 - timeOffset;

	  fprintf (outfile,
		   "3:%d:%d:%d:%d:%lld:%lld:%d:%d:%d:%d:%lld:%lld:%s\n", cpu,
		   appl, task, thread, time_1, time_2, cpu_2, appl_2, task_2,
		   thread_2, time_3, time_4, line);

	  break;

	default:
	  break;
	}

      /* Read one more record is possible */
      if (feof (infile))
	end_read = 1;
      else
	fgets (trace_header, MAX_TRACE_HEADER, infile);
    }

  fclose (infile);
  fclose (outfile);
  unlink (nameIn);

}



void
cutter_copy_file (char *in, char *out)
{

  FILE *fileIn, *fileOut;
  char line[2048];

  if ((fileIn = fopen (in, "r")) == NULL)
    return;
  fileOut = fopen (out, "w");

  while (fgets (line, sizeof (line), fileIn) != NULL)
    fputs (line, fileOut);

  fclose (fileIn);
  fclose (fileOut);

}

/* Function for filtering tasks in cut */
int
cutter_is_selected_task (int task_id)
{

  int i;

  for (i = 0; i < MAX_SELECTED_TASKS; i++)
    {
      if (wanted_tasks[i].min_task_id == 0)
	break;

      if (wanted_tasks[i].range)
	{
	  if (task_id >= wanted_tasks[i].min_task_id
	      && task_id <= wanted_tasks[i].max_task_id)
	    return 1;
	}
      else if (task_id == wanted_tasks[i].min_task_id)
	return 1;
    }

  return 0;
}


int
cutter_main (int argc, char *argv[])
{
  FILE *log_file;
  char *envptr, *c, *tmp_dir, *word, *trace_header;
  char trace_name[1024], buffer[1024], end_parsing = 0, reset_counters;
  char trace_file_out[2048];

  unsigned int id, cpu, appl, task, thread, state, cpu_2, appl_2, task_2,
    thread_2, size, tag;
  unsigned long long type, value, time_1, time_2, time_3, time_4;
  int i, j, k, end_line;

  unsigned long num_iters = 0;
  struct thread_info *p;

  /* Ini Data */
  for (i = 0; i < MAX_APPL; i++)
    for (j = 0; j < MAX_TASK; j++)
      for (k = 0; k < MAX_THREAD; k++)
	tasks[i][j][k] = NULL;

  cutter_last_showed = 0;

  min_perc = 0;
  max_perc = 100;
  keep_events = 0;
  total_cutter_iters = 0;
  by_time = 0;
  old_times = 0;
  max_size = 0;
  cut_tasks = 0;
  break_states = 1;
  is_zip = 0;
  init_task_counter = 0;
  usefull_tasks = 0;
  first_time_caught = 0;
  num_tasks = 0;
  current_size = 0;

  first = NULL;
  last = NULL;

  for (i = 0; i < MAX_SELECTED_TASKS; i++)
    wanted_tasks[i].min_task_id = 0;

  /* Reading of the program arguments */
  cutter_read_args (argc, argv);
  strcpy (trace_name, argv[1]);

  /* Is the trace zipped ? */
  if ((c = strrchr (trace_name, '.')) != NULL)
    {
      /* The names finishes with .gz */
      if (!strcmp (c, ".gz"))
	is_zip = 1;
      else
	is_zip = 0;
    }

  /* Load what counters appears in the trace */
  reset_counters = 0;
  load_counters_of_pcf (trace_name);

  /* Open the files.  If NULL is returned there was an error */
  if (!is_zip)
    {
#if defined(OS_FREEBSD)
      if ((infile = fopen (trace_name, "r")) == NULL)
	{
	  perror ("ERROR");
	  printf ("Cutter: Error Opening File %s\n", trace_name);
	  exit (1);
	}
#else
      if ((infile = fopen64 (trace_name, "r")) == NULL)
	{
	  perror ("ERROR");
	  printf ("Cutter: Error Opening File %s\n", trace_name);
	  exit (1);
	}
#endif
    }
  else
    {

      if ((gzInfile = gzopen (trace_name, "rb")) == NULL)
	{
	  printf ("Cutter: Error opening compressed trace\n");
	  exit (1);
	}

    }

  /* Copy of the .pcf */
  char *pcfIn, *pcfOut;

  pcfIn = strdup (trace_name);
  pcfOut = strdup (argv[2]);
  pcfIn[strlen (pcfIn) - 4] = '\0';
  pcfOut[strlen (pcfOut) - 4] = '\0';
  sprintf (pcfIn, "%s.pcf", pcfIn);
  sprintf (pcfOut, "%s.pcf", pcfOut);
  cutter_copy_file (pcfIn, pcfOut);
  free (pcfIn);
  free (pcfOut);

  /* Copiem el .row */
  char *rowIn, *rowOut;

  rowIn = strdup (trace_name);
  rowOut = strdup (argv[2]);
  rowIn[strlen (rowIn) - 4] = '\0';
  rowOut[strlen (rowOut) - 4] = '\0';
  sprintf (rowIn, "%s.row", rowIn);
  sprintf (rowOut, "%s.row", rowOut);
  cutter_copy_file (rowIn, rowOut);
  free (rowIn);
  free (rowOut);

  /* Put the info in the log file */
  envptr = getenv ("CUTTER_LOGFILE");
  if (envptr != NULL)
    {

      if ((log_file = fopen (envptr, "a")) == NULL)
	{
	  printf ("Can't open log file %s\n", envptr);
	  exit (1);
	}

      printf ("\nUpdating log file...");
      if (by_time)
	fprintf (log_file,
		 "%s.prv:\tFrom  %s.prv\tMin_time: %lld\t Max_time: %lld\n\n",
		 argv[2], argv[1], time_min, time_max);
      else
	fprintf (log_file,
		 "%s.prv:\tFrom  %s.prv\tMin_percentage: %u\t Max_percentage: %u\n\n",
		 argv[2], argv[1], min_perc, max_perc);

      fclose (log_file);

    }

  if (!break_states)
    {

      if ((tmp_dir = getenv ("TMPDIR")) == NULL)
	tmp_dir = getenv ("PWD");

      sprintf (trace_file_out, "%s/tmp_fileXXXXXX", tmp_dir);
      mkstemp (trace_file_out);
    }
  else
    strcpy (trace_file_out, argv[2]);

#if defined(OS_FREEBSD)
  if ((outfile = fopen (trace_file_out, "w")) == NULL)
    {
      printf ("Error Opening Cutter Ouput File %s\n", trace_file_out);
      exit (1);
    }
#else
  if ((outfile = fopen64 (trace_file_out, "w")) == NULL)
    {
      printf ("Error Opening Cutter Ouput File %s\n", trace_file_out);
      exit (1);
    }
#endif

  printf ("\nGenerating trace %s...", argv[2]);
  fflush (stdout);

  ini_cutter_progress_bar (trace_name);

  /* Process header */
  trace_header = (char *) malloc (sizeof (char) * MAX_TRACE_HEADER);
  if (!is_zip)
    fgets (trace_header, MAX_TRACE_HEADER, infile);
  else
    {
      gzgets (gzInfile, trace_header, MAX_TRACE_HEADER);
    }

  proces_cutter_header (trace_header, argv[1], argv[2]);
  free (trace_header);

  /* We process the trace like the old_times version */
  if (!break_states)
    old_times = 1;

  /* Processing the trace records */
  while (!end_parsing)
    {

      /* Read one more record is possible */
      if (!is_zip)
	{
	  if (feof (infile) || fgets (line, sizeof (line), infile) == NULL)
	    {
	      end_parsing = 1;
	      continue;
	    }
	}
      else
	{
	  if (gzeof (gzInfile))
	    {
	      end_parsing = 1;
	      continue;
	    }
	  gzgets (gzInfile, line, sizeof (line));
	}

      if (num_iters == total_cutter_iters)
	{
	  show_cutter_progress_bar ();
	  num_iters = 0;
	}
      else
	num_iters++;

      /* 1: state; 2: event; 3: comm; 4: global comm */
      switch (line[0])
	{

	case '1':

	  sscanf (line, "%d:%d:%d:%d:%d:%lld:%lld:%d\n", &id, &cpu, &appl,
		  &task, &thread, &time_1, &time_2, &state);

	  /* If is a not traceable thread, get next record */
	  if (cut_tasks && !cutter_is_selected_task (task))
	    break;

	  if (time_2 <= time_min)
	    break;

	  if (time_1 < time_min && time_2 >= time_min && remFirstStates)
	    break;

	  if (time_1 < time_max && time_2 > time_max && remLastStates)
	    break;

	  if (old_times && time_1 > time_max)
	    {
	      if (tasks[appl - 1][task - 1][thread - 1]->finished)
		{
		  usefull_tasks--;
		  tasks[appl - 1][task - 1][thread - 1]->finished = 0;
		}
	      break;
	    }

	  if (!old_times && time_1 > time_max)
	    {
	      fclose (outfile);

	      if (!is_zip)
		fclose (infile);
	      else
		gzclose (gzInfile);

	      printf ("...Done\n\n");
	      return 0;
	    }

	  if (tasks[appl - 1][task - 1][thread - 1] == NULL)
	    {
	      if ((p =
		   (struct thread_info *)
		   malloc (sizeof (struct thread_info))) == NULL)
		{
		  perror ("No more memory!!!\n");
		  exit (1);
		}
	      if (first == NULL)
		{
		  first = p;
		  p->previous = NULL;
		}

	      p->next = NULL;
	      if (last == NULL)
		last = p;
	      else
		{
		  p->previous = last;
		  last->next = p;
		  last = p;
		}

	      p->appl = appl;
	      p->task = task;
	      p->thread = thread;

	      tasks[appl - 1][task - 1][thread - 1] = p;
	      num_tasks++;
	      usefull_tasks++;
	      init_task_counter = 1;
	      p->finished = 1;
	      tasks[appl - 1][task - 1][thread - 1]->first = 0;
	      tasks[appl - 1][task - 1][thread - 1]->last = -1;

	      /* Have to reset HC and the begining of cut */
	      reset_counters = 1;

	    }

	  if (!old_times)
	    {

	      if (time_2 >= time_max)
		time_2 = total_time;
	      else
		time_2 = time_2 - time_min;

	      if (time_1 <= time_min)
		time_1 = 0;
	      else
		time_1 = time_1 - time_min;
	    }
	  tasks[appl - 1][task - 1][thread - 1]->last_time = time_2;

	  if (!first_time_caught)
	    {
	      first_record_time = time_1;
	      first_time_caught = 1;
	    }
	  else
	    {
	      if (time_1 < first_record_time)
		first_record_time = time_1;
	    }

	  last_record_time = time_2;

	  current_size +=
	    fprintf (outfile, "%d:%d:%d:%d:%d:%lld:%lld:%d\n", id, cpu, appl,
		     task, thread, time_1, time_2, state);

	  if (reset_counters)
	    {
	      reset_counters = 0;
	      sprintf (line, "2:%d:%d:%d:%d:%lld", cpu, appl, task, thread,
		       time_1);

	      for (i = 0; i < last_counter; i++)
		sprintf (line, "%s:%lld:0", line, counters[i]);

	      if (i > 0)
		current_size += fprintf (outfile, "%s\n", line);
	    }

	  break;

	case '2':
	  sscanf (line, "%d:%d:%d:%d:%d:%lld:%s\n", &id, &cpu, &appl, &task,
		  &thread, &time_1, buffer);
	  strcpy (line, buffer);

	  /* If isn't a traceable thread, get next record */
	  if (cut_tasks && !cutter_is_selected_task (task))
	    break;

	  /* If time out of the cut, exit */
	  /*                  if(time_1 > time_max && !old_times) {
	     fclose(infile); fclose(outfile);
	     if(is_zip) {
	     sprintf(line,"rm %s/tmp.prv",tmp_dir);
	     system(line);
	     }

	     printf("...Done\n\n");
	     exit(0);
	     }
	   */
	  if (tasks[appl - 1][task - 1][thread - 1] != NULL
	      && time_1 > tasks[appl - 1][task - 1][thread - 1]->last_time
	      && time_1 > time_max)
	    break;

	  if (tasks[appl - 1][task - 1][thread - 1] == NULL
	      && time_1 > time_max)
	    break;

	  if (tasks[appl - 1][task - 1][thread - 1] == NULL && remFirstStates)
	    break;

	  /* If time inside cut, adjust time */
	  if ((time_1 >= time_min && time_1 <= time_max)
	      || (time_1 < time_min
		  && tasks[appl - 1][task - 1][thread - 1] != NULL
		  && tasks[appl - 1][task - 1][thread - 1]->last_time >=
		  time_min && keep_events) || (time_1 > time_max
					       && tasks[appl - 1][task -
								  1][thread -
								     1] !=
					       NULL
					       && tasks[appl - 1][task -
								  1][thread -
								     1]->
					       last_time >= time_1
					       && keep_events))
	    {
	      if (!old_times)
		time_1 = time_1 - time_min;

	      if (time_1 > last_record_time)
		last_record_time = time_1;
	      current_size +=
		fprintf (outfile, "%d:%d:%d:%d:%d:%lld:%s\n", id, cpu, appl,
			 task, thread, time_1, line);

	      /* For closing all the opened calls */
	      /* Event type and values */
	      end_line = 0;
	      word = strtok (line, ":");
	      type = atoll (word);
	      word = strtok (NULL, ":");
	      value = atoll (word);

	      update_queue (appl - 1, task - 1, thread - 1, type, value);

	      while (!end_line)
		{

		  if ((word = strtok (NULL, ":")) != NULL)
		    {
		      type = atoll (word);
		      word = strtok (NULL, ":");
		      value = atoll (word);
		      update_queue (appl - 1, task - 1, thread - 1, type,
				    value);
		    }
		  else
		    end_line = 1;
		}

	    }
	  break;

	case '3':
	  sscanf (line,
		  "%d:%d:%d:%d:%d:%lld:%lld:%d:%d:%d:%d:%lld:%lld:%d:%d\n",
		  &id, &cpu, &appl, &task, &thread, &time_1, &time_2, &cpu_2,
		  &appl_2, &task_2, &thread_2, &time_3, &time_4, &size, &tag);

	  /* If isn't a traceable thread, get next record */
	  if (cut_tasks && !cutter_is_selected_task (task)
	      && !cutter_is_selected_task (task_2))
	    break;

	  if (time_1 >= time_min)
	    {

	      /* if time outside the cut, finish */
	      if (time_1 > time_max && !old_times)
		{
		  fclose (outfile);
		  if (!is_zip)
		    fclose (infile);
		  else
		    gzclose (gzInfile);

		  printf ("...Done\n\n");
		  return 0;

		}

	      if (time_4 <= time_max && time_2 <= time_max)
		{

		  if (!old_times)
		    {
		      time_1 = time_1 - time_min;
		      time_2 = time_2 - time_min;
		      time_3 = time_3 - time_min;
		      time_4 = time_4 - time_min;
		    }

		  last_record_time = time_3;

		  current_size +=
		    fprintf (outfile,
			     "%d:%d:%d:%d:%d:%lld:%lld:%d:%d:%d:%d:%lld:%lld:%d:%d\n",
			     id, cpu, appl, task, thread, time_1, time_2,
			     cpu_2, appl_2, task_2, thread_2, time_3, time_4,
			     size, tag);

		}
	    }
	  break;

	case '4':
	  sscanf (line, "%d:%d:%d:%d:%d:%lld:%s\n", &id, &cpu, &appl, &task,
		  &thread, &time_1, buffer);
	  strcpy (line, buffer);

	  /* If is a not traceable thread, get next record */
	  if (cut_tasks && !cutter_is_selected_task (task))
	    break;

	  /* If time out of the cut, exit */
	  if (time_1 > time_max && !old_times)
	    {
	      fclose (outfile);
	      if (!is_zip)
		fclose (infile);
	      else
		gzclose (gzInfile);

	      printf ("...Done\n\n");
	      return 0;
	    }

	  if (old_times)
	    current_size +=
	      fprintf (outfile, "%d:%d:%d:%d:%d:%lld:%s\n", id, cpu, appl,
		       task, thread, time_1, line);
	  else
	    {
	      if (time_1 >= time_min && time_1 <= time_max)
		{
		  time_1 = time_1 - time_min;
		  current_size +=
		    fprintf (outfile, "%d:%d:%d:%d:%d:%lld:%s\n", id, cpu,
			     appl, task, thread, time_1, line);
		}
	    }
	  break;
	}

      if (max_size > 0)
	if (max_size <= current_size)
	  break;

      if (init_task_counter && usefull_tasks == 0)
	break;
    }

  printf ("...Done\n\n");
  fflush (stdout);

  if (!break_states)
    old_times = 0;

  if (old_times)
    {
      printf ("Filling trace with additional records...");

      adjust_to_final_time ();

      printf ("...Done\n");
    }

  /* Close the files */
  fclose (outfile);
  if (!is_zip)
    fclose (infile);
  else
    gzclose (gzInfile);

  if (!break_states)
    {
      printf ("Shifting trace times...");
      shift_trace_to_zero (trace_file_out, argv[2]);
      printf ("...Done\n\n");
    }


  return 0;
}
