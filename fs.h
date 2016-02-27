



#define DIRSIZE	14

struct dirent {
	uint16_t inum;
	char name[DIRSIZE];
};

#define T_DIR  1   // Directory
#define T_FILE 2   // File
#define T_DEV  3   // Device


#define D_INITDR 1 // Initdr ramdsk

// table mapping major device number to
// device functions
struct devsw {
  int (*read)(struct inode*, char*, int);
  int (*write)(struct inode*, char*, int);
};
