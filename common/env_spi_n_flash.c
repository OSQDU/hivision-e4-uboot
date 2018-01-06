
#include <common.h>
/*#include <config.h>*/
#include <environment.h>
#include <nand.h>
/*#include <asm/arch/platform.h>*/

#ifdef CONFIG_ENV_IS_IN_SPI_FLASH
extern unsigned char sf_env_get_char_spec(int);
extern int sf_env_init(void);
extern int sf_saveenv(void);
extern int sf_env_relocate_spec(void);
extern char *sf_env_name_spec;
#endif

#ifdef CONFIG_ENV_IS_IN_SPI_NAND
extern unsigned char spinand_env_get_char_spec(int);
extern int spinand_env_init(void);
extern int spinand_saveenv(void);
extern int spinand_env_relocate_spec(void);
extern char *spinand_env_name_spec;
#endif

char *env_name_spec;
env_t *env_ptr;

DECLARE_GLOBAL_DATA_PTR;

struct env_spi_func_t {
	unsigned char (*env_get_char_spec)(int);
	int (*env_init)(void);
	int (*saveenv)(void);
	int (*env_relocate_spec)(void);
	char *env_name_spec;
};

struct env_spi_func_t g_env_spi_func;

uchar env_get_char_spec(int index)
{
	return g_env_spi_func.env_get_char_spec(index);
}

int saveenv(void)
{
	return g_env_spi_func.saveenv();
}

#if !defined(ENV_IS_EMBEDDED)
static void use_default(void)
{
	puts ("*** Warning - bad CRC, using default environment\n\n");
	set_default_env();
}
#endif

#ifdef CONFIG_SPI_FLASH
extern struct spi_flash *env_flash;
#endif

void env_relocate_spec(void)
{
	int ret=0;
	if (g_env_spi_func.env_relocate_spec) {
		g_env_spi_func.env_relocate_spec();
		return;
	}
	/* identify spi-nor or spi-nand*/
#ifdef CONFIG_SPI_FLASH
	ret = sf_env_relocate_spec();
	if (env_flash) {/* spi-nor */
		g_env_spi_func.saveenv = sf_saveenv;
		g_env_spi_func.env_name_spec = sf_env_name_spec;
		g_env_spi_func.env_get_char_spec = sf_env_get_char_spec;
		g_env_spi_func.env_relocate_spec = sf_env_relocate_spec;
		env_name_spec = sf_env_name_spec;
		if (ret==0)
			debug("Load env from spi nor flash\n");
		else
			use_default();
		return;
	}
#endif

#ifdef CONFIG_FH_SPI_NAND
#ifdef CONFIG_ENV_IS_IN_SPI_NAND
	ret = spinand_env_relocate_spec();
	if ( nand_info->priv ) {/* spi-nand */
		g_env_spi_func.saveenv = spinand_saveenv;
		g_env_spi_func.env_name_spec = spinand_env_name_spec;
		g_env_spi_func.env_get_char_spec = spinand_env_get_char_spec;
		g_env_spi_func.env_relocate_spec = spinand_env_relocate_spec;

		env_name_spec = spinand_env_name_spec;
		if (ret==0)
			debug("Load env from spi nand flash\n");
		else
			use_default();
		return;
	}
#endif
#endif
	use_default();
}

int env_init(void)
{
	/*TODO: check spi flash type from gpio?
	 * set env func here
	 * */
	int ret = -1;
#ifdef CONFIG_SPI_FLASH
	ret = sf_env_init();
#endif
#ifdef CONFIG_FH_SPI_NAND
	ret = spinand_env_init();
#endif
	return ret;
}
