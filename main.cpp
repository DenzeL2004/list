#include <stdio.h>

#include "list.h"
#include "src/log_info/log_errors.h"
#include "src/Generals_func/generals.h"

int main ()
{
    #ifdef USE_LOG
        
        if (Open_logs_file ())
            return OPEN_FILE_LOG_ERR;

    #endif 

    List list = {};

    if (List_ctor (&list))
    {
        Log_report ("ERROR: Ctor list in main\n");
        Err_report ();
        return -1;
    }

    List_dump (&list, "AFTER CTOR");

    for (int i = 0; i <= 5; i++){
        List_insert_befor_ptr (&list, list.root);
    }
    
    List_dump (&list, "FROM MAIN");

    for (int i = 3; i >= 1; i--){
        List_erase (&list, Get_pointer_by_logical_index (&list, 1));
    }

    List_dump (&list, "FROM MAIN");

    if (List_dtor (&list))
    {
        Log_report ("ERROR: Dtor list in main\n");
        Err_report ();
        return -1;
    }
    
    #ifdef USE_LOG
        
        if (Close_logs_file ())
            return CLOSE_FILE_LOG_ERR;

    #endif
}