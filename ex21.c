#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int compare_files(const char* filename1, const char* filename2) {
    // Get the sizes of the files
	int sim = 0;
    // Open the files for reading
    int fd1 = open(filename1, O_RDONLY);
    int fd2 = open(filename2, O_RDONLY);
    if (fd1 < 0 || fd2 < 0) {
        write(2,"Erro in: open\n",15);
        return -1; // Error in opening file
    }

    // Read and compare the files in parallel
	int c1=0,c2=0,read1=1,read2=1;
    char buf1, buf2;
    while (read(fd1, &buf1, 1) > 0 && read(fd2, &buf2, 1) > 0) {
		if (buf1 != buf2 || c1 != c2){
			sim=1;
		}
		while ( read1 && (buf1 == ' ' || buf1 == '\n'|| buf1 == '\r')) {
            c1+=1;
            if(read(fd1, &buf1, 1) <= 0){
                read1=0;
            }
        }
		while (read2 && (buf2 == ' '  || buf2 == '\n'|| buf2 == '\r')) {
            c2+=1;
            if(read(fd2, &buf2, 1) <= 0){
                read2=0;
            }
        }
        if (c1 != c2 || read1!=read2){
			sim=1;
		}
        if (buf1 >= 'A' && buf1 <= 'Z'){
            buf1 += 32;
        }
        if (buf2 >= 'A' && buf2 <= 'Z'){
            buf2 += 32;
        }
        if (buf2!=buf1){
            close(fd1);
            close(fd2);
            return 2;
        }
    }

	if (read(fd1, &buf1, 1) > 0){
        sim=1;
        if (buf1 != ' '  && buf1 != '\n' && buf1 != '\r') {
            return 2;
        }
    }
    else if (read(fd2, &buf2, 1) > 0){
        sim=1;
        if (buf2 != ' '  && buf2 != '\n' && buf2 != '\r') {
             return 2;
        }
    }

	// Close the files
    close(fd1);
    close(fd2);

	
	if (sim == 0){
		return 1;// Files are the same
	}

    return 3; 
}


int main(int argc, char *argv[]) {
    int res =  compare_files(argv[1],argv[2]);
    return res;
}