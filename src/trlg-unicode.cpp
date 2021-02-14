#include <sys/types.h>

int convert_wide_char(u_int32_t wchr){
	switch(wchr){
		case 4314:{
			return 'l';
		}break;
		case 4321:{
			return 's';
		}break;
		case 4304:{
			return 'a';
		}break;
		case 4322:{
			return 't';
		}break;
		case 4307:{
			return 'd';
		}break;
		case 4332:{
			return 'w';
		}break;
		case 4306:{
			return 'g';
		}break;
		case 4308:{
			return 'e';
		}break;
		case 4318:{
			return 'p';
		}break;
		case 4323:{
			return 'u';
		}break;
		case 4325:{
			return 'q';
		}break;
		case 4310:{
			return 'z';
		}break;
		case 4334:{
			return 'x';
		}break;
		case 4336:{
			return 'h';
		}break;
		case 4324:{
			return 'f';
		}break;
		case 4330:{
			return 'c';
		}break;
		case 4331:{
			return 'Z';
		}break;
		default:
			return wchr;
		break;
	}
	return 0;
}

