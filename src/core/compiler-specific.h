#ifndef _COMPILER_SPECIFIC_H
#define _COMPILER_SPECIFIC_H

// compiler_warning.h 
#define STRINGISE_IMPL(x) #x 
#define STRINGISE(x) STRINGISE_IMPL(x) 
 
// Use: #pragma message WARN("My message") 
#if _MSC_VER 
#   define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : " 
#   define WARN(exp) (FILE_LINE_LINK "WARNING: " exp) 
#else//__GNUC__ - may need other defines for different compilers 
#   define WARN(exp) ("WARNING: " exp) 
#endif

#endif