#include <sys/types.h>
#include <algorithm>
#include <iostream>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fstream>
#include <vector>
#include <string>
#include <map> 

#include "io.h"

using namespace std;
using namespace ioh;

namespace matchfiles{
	
typedef struct CKL {
	string name;
	int idx; 
} Ckl, *Cklptr; 

typedef struct DIRECTORY {
	char name[260];
} Dir, *Dirptr; 

int N;
int DirNum = 0;
int MarkDown = 0;

Dirptr DirL;
map<string, int> sflag;

vector<string> files;
vector<string> found;
vector<Ckl> missfiles;
vector<Ckl> checkoutQ;

int *flag;

int cmp1(string &a, string &b) {
	return a > b;
}

int cmp2(Ckl &a, Ckl &b) {
	return a.name > b.name;//strcmp(a.name, b.name) >= 0;
}

void Rolling_Buffer(char *buffer, int buffersize, FILE *from, FILE *to, int totalen) {
	int lsize = 0;
	for(int j = 0; j < totalen / buffersize; j++) {
		if(lsize = fread(buffer, 1, buffersize, from)) {
			fwrite(buffer, 1, lsize, to);
		}
	}
	if(lsize = fread(buffer, 1, totalen % buffersize, from)) {
		fwrite(buffer, 1, lsize, to);
	}
}

int copyfile(const char *currentdir, const char *filename) {
	FILE *source = NULL;
	FILE *output = NULL;
	
	int last = 0;//size of a file like 1024B or 1KB 
	char buffer[1024] = {0};
	
	string sourcedir = string(currentdir) + string(filename);
	string desdir = "found\\" + string(filename);
	
	source = fopen(sourcedir.c_str(), "rb");
	if(!source) {
		clearerr(source);
		fclose(source);
		//cout << "can't open " << sourcedir << endl;
		return errno;
	}
	
	output = fopen(desdir.c_str(), "wb");
	if(!output) {
		clearerr(output);
		fclose(output);
		//cout << "can't open " << desdir << endl;
		return errno;
	}
	
	fseek(source, 0, 2);
	last = ftell(source);
	fseek(source, 0, 0);
	Rolling_Buffer(buffer, 1024, source, output, last);
	cout << filename << " has been copied" << endl;
	
	clearerr(source);
	clearerr(output);
	fclose(source);
	fclose(output);
	
	return 0;
}

int match(const char *currentdir, vector<Ckl> &ckol, int strtmatch, int markdown, vector<Ckl> &miss) {
	
	string checkname;
	string filename;
	int i = strtmatch, j = markdown;
	int checkidx;
	
	while(i < ckol.size() && j < files.size()) {
		filename = files[j];
		checkidx = ckol[i].idx;
		checkname = ckol[i].name;
		
		if(flag[checkidx] != 1 && sflag[checkname] != 1) {//CID来自相同型号或批号的产品,只匹配最近CID的报关单;CID相同,但来自不同型号批号的产品,也只拷贝一份
			
			if(filename.find(checkname) != string::npos) {// 遍历文件名池，一一与当前待查找的文件名进行匹配 
				//cout << checkname << " " << ckol[i].idx << " " << flag[ckol[i].idx] << endl;
				found.push_back(checkname);
				flag[checkidx] = 1;
				sflag[checkname] = 1;
				copyfile(currentdir, filename.c_str());

				int t = j + 1;
				//接着往后找  
				while(t < files.size()) {// to match further 
					string rest = files[t];
					if(rest.find(checkname) != string::npos) {
						copyfile(currentdir, rest.c_str());
						j = t;
					}
					else {//一旦后一项的文件全名不包括待查询的CID号
						j = t - 1;
						break;
					}
					t ++;
				}
				i ++;
				MarkDown = j;
			}
			j ++;// 从文件池中取出下一个
		}
		else if(sflag[checkname] == 1) {
			flag[checkidx] = 1;
			i ++;
		}
		else i ++;
	}

	if(i < ckol.size() && j >= files.size()) {
		miss.push_back(ckol[i]); //cout << ckol[i].name << " " << i << endl;
		return i + 1;//当前这个待查找的文件名未查询到任何信息，就先返回
	}
	else return i;
}

int getdir (const char *directory, vector<string> &vec) {
    DIR *dp;
    struct dirent *dirp;

	cout << "now open " << directory << endl;
	
    if((dp = opendir(directory)) == NULL) { // directory doesn't exist
      	cout << "Error(" << errno << ") opening " << directory << endl;
      	return errno;
    }
    while ((dirp = readdir(dp)) != NULL) { // enum all files within the directory
    	string filename = dirp->d_name;
    	cout<< filename << endl; 
    	if(filename.length() >= 3) vec.push_back(filename);
	}
	
	closedir(dp);
    return 0;
}

int initialDirList() {
	fstream fp;
	fp.open("DirectoryList.txt", ios::in);
	
	if(fp) {
		fp >> DirNum;
		DirL = (Dir *)calloc(DirNum, sizeof(Dir));
		
		int i = 0;
		string tmp;
		while(getline(fp, tmp, '\n')) {
			if(tmp.size() >= 1) {
				//cout<<tmp<<endl;
				strcpy((DirL + i)->name, tmp.c_str());
				i ++;
			}
			if(i >= DirNum) break;
		}
		
		cout << "The List of all directories has been successfully initialized\n" << endl;
		fp.clear(); 
		fp.close();
		return 0;
	}
	fp.clear();
	fp.close();
	cout << "something went wrong during input" << endl;
	cout << "check if you place DirectoryList.txt under the current directory" << endl;
	return errno;
}

int initialCheckOutList() {
	int maxidx = 0;
	Input_From_File("CheckOutList.txt");
	
	scanf("%d %d\n", &N, &maxidx);
	printf("待查询的文件名个数: %d\n", N);
	
	flag = (int *)calloc(maxidx + 1, sizeof(int));
	memset(flag, 0, maxidx + 1);
	
	for(int i = 0; i < N; i++) {
		Ckl tmp;
		string ss;
		int idx;
		cin >> ss >> idx;
		tmp.idx = idx, tmp.name = ss;
		//cout << tmp.name << " " << tmp.idx << endl; 
		checkoutQ.push_back(tmp); 
	}
	Close_In();
	
	sort(checkoutQ.begin(), checkoutQ.end(), cmp2);
	cout << "The pending List has been successfully initialized\n"<< endl;
	
	return 0;
}

int start() {
	int isEnd = 0;
	int fd_stdin = 0, fd_stdout = 0;
	
	fd_stdin = dup(fileno(stdin));
	fd_stdout = dup(fileno(stdout));
	
	initialDirList();
	initialCheckOutList();
	
	for(int i = 0; i < DirNum; i++) {
		isEnd = 0, MarkDown = 0;
		
    	getdir((DirL + i)->name, files);// get all filenames stored under a directory
    	sort(files.begin(), files.end(), cmp1);
    	
	    while(isEnd < checkoutQ.size()) {
	    	isEnd = match((DirL + i)->name, checkoutQ, isEnd, MarkDown, missfiles);
		}
		
		if(missfiles.empty()) break;
		else {
			checkoutQ.assign(missfiles.begin(), missfiles.end());
			sort(checkoutQ.begin(), checkoutQ.end(), cmp2);
			missfiles.clear();
		}
		
		files.clear();//clear all and get prepared for the next directory
	}
    
	Output_To_File("missfiles.txt");
	//cout << "请协助提供以下CID对应的报关单，十分感谢!" << endl;
	for(int i = 0; i < checkoutQ.size(); i++) {
		int idx = checkoutQ[i].idx;
		string sdx = checkoutQ[i].name;
		if(flag[idx] != 1 && sflag[sdx] != 1) {
			flag[idx] = 1;
			sflag[sdx] = 1;
			cout << checkoutQ[i].name << endl;
		}
	}

	Output_To_File("foundfiles.txt");
	for(int i = 0; i < found.size(); i++) {
		cout << found[i] << endl;
	}
	Close_Out();
	
	fdopen(fd_stdin, "r");
	fdopen(fd_stdout, "w");
	cout << "\n查找完成，请按任意键结束" << endl;
	getchar();
	
	return 0;
}

}

using namespace matchfiles;

int main() {
	start();
	return 0;
}
