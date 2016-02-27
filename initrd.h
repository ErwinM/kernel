// initrd uses a propriotary (=made up) file system
// 4 bytes stores the nr of files in the system
// 64 headers follow (see def below)
// file contents
// nr of files  |  64 headers  |  data

struct initrdsb {
	uint32_t nfiles;
};

struct initrdhdr
{
   uint8_t magic; 	// The magic number is there to check for consistency.
   char name[64];   // name of the file
 	 uint32_t offset; // Offset in the initrd the file starts.
   uint32_t sz; 		// Length of the file.
};
