#include "ref_import.h"
#include "tr_local.h"

void R_LoadBMP(const char *name, byte **pic, int *width, int *height);
void R_LoadJPG(const char *name, byte **pic, int *width, int *height);
void R_LoadPCX(const char *name, byte **pic, int *width, int *height);
void R_LoadPNG(const char *name, byte **pic, int *width, int *height);
void R_LoadTGA(const char *name, byte **pic, int *width, int *height);

// Description:  Loads any of the supported image types into
// a cannonical 32 bit format.

typedef struct {
	char *ext;
	void (*ImageLoader)(const char *, unsigned char **, int *, int *);
} imageExtToLoaderMap_t;

// Note that the ordering indicates the order of preference used
// when there are multiple images of different formats available
static const imageExtToLoaderMap_t imageLoaders[6] = {{"png", R_LoadPNG},  {"tga", R_LoadTGA}, {"jpg", R_LoadJPG},
													  {"jpeg", R_LoadJPG}, {"pcx", R_LoadPCX}, {"bmp", R_LoadBMP}};

static const int numImageLoaders = 6;

void R_LoadImage(const char *name, unsigned char **pic, int *width, int *height) {
	int orgLoader = -1;
	int i;
	char localName[128] = {0};
	// int len = strlen(name);

	const char *pSrc = name;
	char *pDst = localName;
	// char* dot = NULL;
	char *pExt = NULL;
	// char* slash = NULL;
	char c;

	*pic = NULL;
	*width = 0;
	*height = 0;


	// copy name to localName
	while ((c = *pDst++ = *pSrc++)) {

		if (c == '.')
			pExt = pDst;
		//        else if(c == '/')
		//            slash = pDst-1;
	}

	if (pExt != NULL) {
		// Look for the correct loader and use it
		for (i = 0; i < numImageLoaders; i++) {
			if (!Q_stricmp(pExt, imageLoaders[i].ext)) {
				orgLoader = i;

				// Load
				imageLoaders[i].ImageLoader(localName, pic, width, height);
				if (*pic != NULL) {
					// Something loaded
					return;
				}
			}
		}

		// Loader failed, most likely because the file isn't there;
		// Try and find a suitable match using all the image formats supported

		for (i = 0; i < numImageLoaders; i++) {
			if (i == orgLoader)
				continue;

			strcpy(pExt, imageLoaders[i].ext);

			// Load
			imageLoaders[i].ImageLoader(localName, pic, width, height);

			if (*pic != NULL) {
				// Something loaded
				// ri.Printf( PRINT_WARNING, "%s not present, using %s instead\n", name, localName );
				return;
			}
		}

		ri.Printf(PRINT_WARNING, "%s not present\n", localName);

	} else {

		// Try and find a suitable match using all the image formats supported
		*(pDst - 1) = '.';

		for (i = 0; i < numImageLoaders; i++) {
			strcpy(pDst, imageLoaders[i].ext);
			// Load
			imageLoaders[i].ImageLoader(localName, pic, width, height);

			if (*pic != NULL) {
				ri.Printf(PRINT_WARNING, "%s without a extension, using %s instead. \n", name, localName);
				return;
			}
		}
	}

	ri.Printf(PRINT_WARNING, "%s not present.\n", name);

	// try again without the extension
}
