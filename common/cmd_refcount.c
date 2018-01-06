#include <common.h>
#include <command.h>
#include <spi_flash.h>
#include <hush.h>
#ifdef CONFIG_CMD_REFCOUNT

#define DEFAULT_MODE		0
#define ADD_MODE		1
#define GET_MODE		2
#define COMPARE_MODE	3




char *itoa(unsigned int i)
{
	/* 21 digits plus null terminator, good for 64-bit or smaller ints */
	static char local[22];
	char *p = &local[21];
	*p-- = '\0';
	do {
		*p-- = '0' + i % 10;
		i /= 10;
	} while (i > 0);
	return p + 1;
}


int do_refcount(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{

	static unsigned int mode_flag = DEFAULT_MODE;
	//char *endp;
	char *env_name;
	char *temp_char;
	unsigned int temp_uint;
	int ret;
	ulong add_value;
	ulong get_cmp_value;
	ulong old_value;
	int flag_cmd=0;
	//cmd = simple_strtoul(argv[1], &endp, 0);
//	struct spi_flash *flash;

//	flash = spi_flash_probe(1,1,1,1);
//	if(flash == NULL) {
//		printf("get flash error..\n ");
//		return -1;
//	}

	char* cmd_env1;
	char* cmd_env2;






	if (strcmp (argv[1], "add") == 0){
		mode_flag = ADD_MODE;
	}
	else if (strcmp (argv[1], "get") == 0){
		mode_flag = GET_MODE;
	}
	else if (strcmp (argv[1], "compare") == 0){
		mode_flag = COMPARE_MODE;
	}
	else {
		printf("error cmd...\n");
		return -1;
	}

	env_name = getenv ("refcount");
	if(!env_name){
		ret = setenv("refcount","0");
		if(ret !=0){
			printf("set env error...\n");
			return -1;
		}
		//reget the point of the env named 'refcount';or it will point to de string..
		env_name = getenv ("refcount");
		//run_command ("saveenv", flag_cmd);
	}
	switch(mode_flag){
	case ADD_MODE:
		add_value = simple_strtoul (argv[2], NULL, 0);
		old_value = env_name ? (int)simple_strtol(env_name, NULL, 0) : 20;
		old_value +=add_value;
		temp_uint = (unsigned int)old_value;
		temp_char = itoa(temp_uint);
		setenv("refcount",temp_char);
		break;
	case GET_MODE:
		old_value = env_name ? (int)simple_strtol(env_name, NULL, 0) : -1;
		printf("old value is %d\n",(int) old_value);

		break;
	case COMPARE_MODE:
		old_value = env_name ? (int)simple_strtol(env_name, NULL, 0) : -1;
		get_cmp_value  = simple_strtoul (argv[2], NULL, 0);

		printf("old value is %d\n",(int) old_value);
		printf("cmp value is %d\n",(int) get_cmp_value);

		if(old_value == get_cmp_value){
			cmd_env1 = getenv(argv[3]);
			//printf(cmd_env1);
			if(!cmd_env1){
				printf("can't find env:%s\n",argv[3]);
				return -1;
			}

			run_command (cmd_env1, flag_cmd);
		}
		else{
			cmd_env2 = getenv(argv[4]);
			//printf(cmd_env2);
			if(!cmd_env2){
				printf("can't find env:%s\n",argv[4]);
				return -1;
			}

			run_command (cmd_env2, flag_cmd);
		}
		break;

	default:
		printf("error cmd...\n");
		break;
	}

	return 0;
}

U_BOOT_CMD(
		refcount, 8, 1, do_refcount,
		"refcount utility commands",
		"cmd list:\n"
		"\tadd [number]:refcount += [number]\n"
		"\tget:print the refcount value \n"
		"\tcompare [number] 'env1' 'env2' :if refcount == [number] do env1;else do env2 \n"

);

#endif
