

#ifndef GBEMU_SRC_VERSION_H_
#define GBEMU_SRC_VERSION_H_

#ifndef PROJECT_NAME
#define PROJECT_NAME gbemu
#endif // !PROJECT_NAME

#ifndef MAJOR_VER
#define MAJOR_VER 1
#endif // !MAJOR_VER

#ifndef MINOR_VER
#define MINOR_VER 0
#endif // !MINOR_VER

#ifndef BUILD_VER
#define BUILD_VER 0
#endif // !BUILD_VER


// utility used to stringify tokens
#define XSTR(s) STR(s)
#define STR(s) 	#s


#define PROJECT_NAME_STR XSTR(PROJECT_NAME)
#define VERSION_STR "v" XSTR(MAJOR_VER) "." XSTR(MINOR_VER) "." XSTR(BUILD_VER)


#endif // GBEMU_SRC_VERSION_H_