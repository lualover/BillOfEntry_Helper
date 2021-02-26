#include <stdlib.h>
#include <stdio.h>

#ifndef IO
#define IO

namespace ioh {

	int Input_From_File(const char *path) {

		try {
			setbuf(stdin, NULL);
			FILE *fp = freopen(path, "r", stdin);
			if(!fp) throw "something went wrong during input";
		}
		catch(const char *msg) {
			printf("%s\n", msg);
			printf("check if you place file %s under the current directory\n", path);
			return -1;
		}
		return 0;
	}
	
	int Output_To_File(const char *path) {
		try {
			setbuf(stdout, NULL);
			FILE *fp = freopen(path, "w", stdout);
			if(!fp) throw "something went wrong during output";
		}
		catch(const char *msg) {
			printf("%s\n", msg);
			printf("the file named %s seems denied here\n", path);
			return -1;
		}
		return 0;
	}
	
	void Close_In() {
		setbuf(stdin, NULL);
		fclose(stdin);
	}
	
	void Close_Out() {
		setbuf(stdout, NULL);
		fclose(stdout);
	}

}

#endif 
