#include <assert.h>
#include "config_core.h"
#include "config_example_extension.h"

typedef struct {
	char *output;
	char *input;
	bool vox;
} audio_config;

typedef struct {
	char         *host;
	uint32_t      port;
	stringlist    nicks;
	audio_config  audio;
} config;

static const config_mapping audio_config_map[] = {
	{ "output_device",     's', offsetof(audio_config, output), .sm=NULL }, // string
	{ "input_device",      's', offsetof(audio_config, input),  .sm=NULL }, // string
	{ "voice_auto_detect", 'b', offsetof(audio_config, vox),    .sm=NULL }, // boolean
	{  NULL,                0,  0,                              .sm=NULL }  // end
};
static const config_mapping config_map[] = {
	{ "server_host", 's', offsetof(config, host),  .sm=NULL                                                }, // read a string
	{ "server_port", 'u', offsetof(config, port),  .sm=NULL                                                }, // read an unsigned integer
	{ "nicklist",    '*', offsetof(config, nicks), .op={read_stringlist, write_stringlist, free_stringlist}}, // more complex type with custom functions
	{ "audio",       '+', offsetof(config, audio), .sm=audio_config_map,                                   }, // subconfig
	{  NULL,          0,  0,                       .sm=NULL                                                }
};
int main(void) {

	char *cfg_text = "\n\
		# general configuration\n\
		server_host = example.com\n\
		server_port = 6697\n\
		\n\
                # multiple nicks with special datatype\n\
		nicklist = 3\n\
			lena\n\
			l3na\n\
			len4\n\
		\n\
		# audio configuration goes here\n\
		audio = {\n\
			output_device = default\n\
			input_device = default\n\
			voice_auto_detect = no\n\
		}\n";

	printf("the config string:\n---\n%s\n---\n", cfg_text);
	FILE *tf = tmpfile();
	assert(tf != NULL);
	rewind(tf);
	fprintf(tf, "%s", cfg_text);
	rewind(tf);

	config cfg;
	inflate_config(&cfg, config_map, tf);
	printf("the config as reprinted from the inflated data\n---\n");
	deflate_config(stdout, &cfg, config_map);
	printf("\n---\n");

	free_config_contents(&cfg, config_map);
}
