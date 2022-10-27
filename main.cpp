#include <stdio.h>

#include "list.h"
#include "src/log_info/log_errors.h"

int main ()
{
    #ifdef USE_LOG
        
        if (Open_logs_file ())
            return OPEN_FILE_LOG_ERR;

    #endif 

    List list = {};

    if (List_ctor (&list, 10))
    {
        Log_report ("ERRROR: Ctor list in main\n");
        Err_report ();
        return -1;
    }

    List_dump (&list);

    for (int i = 0; i <= 8; i++){
        List_insert (&list, 0, (i+1)*(i+1));
        List_dump (&list);
    }
    
    
    for (int i = 1; i <= 6; i++){
        List_erase (&list, i);
        List_dump (&list);
    }

    List_insert (&list, 0, 10);
    List_dump (&list);

    if (List_dtor (&list))
    {
        Log_report ("ERRROR: Dtor list in main\n");
        Err_report ();
        return -1;
    }

    #ifdef USE_LOG
        
        if (Close_logs_file ())
            return CLOSE_FILE_LOG_ERR;

    #endif
}