#ifndef _GENERALS_H_
#define _GENERALS_H_

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>	
#include <stdint.h>
#include <fcntl.h>

#define MIN(a, b) ((a) < (b) ? (а) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * @def is_error
 * @brief Checks for false input conditions
 * @note This define does not terminate the program in case of an error like assert
*/
#define is_error(condition) {                                           \
    if (!(condition))                                                   \
        printf ("ERROR in %s in line %d in file %s in function %s\n",   \
                #condition, __LINE__, __FILE__, __PRETTY_FUNCTION__);   \
}

const char RESET[]  =     "\033[0m";     ///< Standart colour
const char RED[]    =  "\033[1;31m";     ///< Red colour
const char BLUE[]   =  "\033[1;36m";     ///< Blue colour
const char GREEN[]  =  "\033[1;32m";     ///< Green colour
const char YELLOW[] =  "\033[1;33m";     ///< Yellow colour

const long double Eps = 1e-10; ///< Global constant equal to \f$\ 10^{-9}\f$

/**
 * @enum Errors
 * @brief Represents the status of errors
*/
enum Errors {
    ERR_MEMORY_ALLOC        = -1,          ///< Memory is not allocated
    ERR_INIT_BUF            = -2,          ///< Buffer has not been initialized
    ERR_FILE_READING        = -3,          ///< Not all data was read from the file
    ERR_INIT_LINES          = -4,          ///< Lines does not initialize
    ERR_TEXT_READING        = -5,          ///< Text does not initialize
    ERR_FILE_OPEN           = -6,          ///< File is not open
    ERR_FILE_CLOSE          = -7,          ///< File is not close
    ERR_WRITING             = -8,          ///< Data writing failed
    ERR_MANY_ARGUMENTS      = -9,          ///< Too many values on the command line
    ERR_PARSING             = -10,         ///< Parsing does not succeeded
    ERR_RE_ClEARING_MEMORY  = -13,
    ERR_NULLPTR             = -11,
};

enum Comparison{
    DIFRENT = -1,
    EQUAL   = 0
};

/**
 * @struct Options
 * @brief Represents the status of flags
*/
struct Options{
    bool read_on_file   = 0;
    const char *file_input_name  = nullptr;

    bool write_on_file  = 0;
    const char *file_output_name = nullptr;

    bool info_option    = 0;
};

bool Check_nullptr  (void *ptr);

bool Check_num (const char *str);

/** 
 * @brief Comparison of double variables
 * @version 1.0.0
 * @param [in] num1 input double parameter
 * @param [in] num2 input double parameter
 * @return False - number is not equal, true - number is equal
*/
bool Equality_double (double num1, double num2);

/** 
 * @brief Comparing a number to zero
 * @version 1.0.0
 * @param [in] n input parameter
 * @return False - number is not zero, true - number is zero
*/
bool Is_zero (double num);

/** 
 * @brief Correct zero value
 * @version 1.0.0
 * @param [in] n input parameter
 * @note If the value is zero the program must not return -0.00
 * @return Returns zero if the number is zero, otherwise returns the number itself
*/
double Fix_zero (double num);

int Bin_represent (FILE *fpout, size_t elem, uint64_t size_elem);

int Clear_data (unsigned char *cmd_hash_tabel, size_t size_data);

uint64_t Get_hash (const char *data, uint64_t len);

/** 
 * @brief Paints a line
 * @version 1.0.0
 * @param [in] colour colour you want to paint a string
 * @param [in] str line to be colored
*/
void Print_colour (char const colour[], char const *str, ...);

/** 
 * @brief Parsing comon line
 * @version 1.0.0
 * @param [in] argc argument count
 * @param [in] argv argument values
 * @param [in] Struct option, which we want to use
 * @return Return zero, if program worked correctly
*/
int Parsing (int argc, const char *argv[], Options *option);

/** 
 * @brief Process options
 * @version 1.0.0
 * @param [in] Struct option, which we want to use
 * @return Return zero, if program worked correctly
*/
int Process_parsing (Options *options);

/**
 * @brief Opens a file in a specific mode
 * @param [in] name_file Name of file 
 * @param [in] mode Open file access modes
 * @return Pointer on file 
 */
FILE *Open_file_ptr (const char *name_file, const char *mode);

/**
 * @brief Close a file
 * @param [in] fp Pointer of file 
 * @return Zero - if closing happened without error 
 */
char Close_file_ptr (FILE *fp);

/**
 * @brief Creat empty file
 * If file exist, clear it
 * @param [in] name_file Name of file 
 * @param [in] mode Open file access modes
 * @return Pointer on file 
 */
int Creat_empty_file (const char *name_file);

int Open_file_discriptor (const char *name_file, const int mode);

char Close_file_discriptor (int fd);

/**
 * @brief Swaps the values of variables
 * @param obj1 [in] first variable
 * @param obj2 [in] second variable
 * @param size_type [in] size of variables in bytes
 * @return Returns zero if the variables have changed values 
 */
int My_swap (void *obj1, void *obj2, size_t size_type);

#endif
