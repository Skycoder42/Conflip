#include "conflipservice.h"

int main(int argc, char *argv[])
{
	ConflipService service{argc, argv};

	for(auto i = 1; i < argc; i++) {
		if(qstrcmp(argv[i], "--slice") == 0) {
			if((i + 1) >= argc)
				return EXIT_FAILURE;
			service.setSlice(QString::fromUtf8(argv[i + 1]));
		}
	}

	return service.exec();
}
