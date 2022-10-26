#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "log_errors.h"
#include "..\Generals_func\generals.h"

static FILE *fp_logs = stderr;

//=======================================================================================================

int Open_logs_file ()
{    
    fp_logs = Open_file_ptr ("logs_info.txt", "a");

    if (!fp_logs)
    {
        fprintf (stderr, "Logs file does not open\n");
        return 0;
    }

    time_t seconds = time (NULL)  + 3 * 60* 60;;   

    fprintf (fp_logs, "\n----------------------------------------------------\n");
    fprintf (fp_logs, "Time open logs file: %s\n\n", asctime(gmtime(&seconds)));

    return 0;
}

//=======================================================================================================

int Log_report_ (const char* file_name, const char* func_name, int line, const char *format, ...) 
{ 
    fprintf (fp_logs, "=================================================\n\n");

    fprintf (fp_logs, "SHORT REFERENCE:\n");

    fprintf (fp_logs, "The program returned an error in\n\n");

    fprintf (fp_logs, "In file %s\n", file_name);
    fprintf (fp_logs, "In function %s\n", func_name);
    fprintf (fp_logs, "In line %d\n\n", line);
    
    va_list args = nullptr;
   
    va_start(args, format);
    vfprintf(fp_logs, format, args);
    va_end(args);

    fprintf (fp_logs, "=================================================\n\n\n");

    return 0;                                                       
}

//=======================================================================================================

int Err_report_ (const char* file_name, const char* func_name, int line) 
{ 

    fprintf (stderr, "||ERROR ERROR ERROR||\n");

    fprintf (stderr, "In file %s, ", file_name);
    fprintf (stderr, "In function %s, ", func_name);
    fprintf (stderr, "In line %d\n\n", line);
    
    return 0;                                                       
}

//=======================================================================================================

FILE *Get_log_file_ptr ()
{
    return fp_logs;
}

//=======================================================================================================

int Close_logs_file ()
{
    time_t seconds = time (NULL)  + 3 * 60* 60;;   

    fprintf (fp_logs, "\n----------------------------------------------------\n");
    fprintf (fp_logs, "Time close logs file: %s\n\n", asctime(gmtime(&seconds)));
    
    if (Close_file_ptr (fp_logs))
    {
        fprintf (stderr, "Logs file does not close\n");
        return ERR_FILE_OPEN;
    }

    return 0;
}