/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <regex.h>
#include <sys/types.h>
#include <media_content.h>
#include <metadata_extractor.h>
#include <mime_type.h>

#include "mf-ug-fs-util.h"
#include "mf-ug-util.h"
#include "mf-ug-file-util.h"

#define MF_UG_PHONE_DEFAULT_LEVEL		2   /*the phone path is /opt/media, it consists of opt and media two parts*/
#define MF_UG_MMC_DEFAULT_LEVEL 		3   /*the mmc path is /opt/storage/sdcard, it consists of opt and storage and sdcard three parts*/
#define CONDITION_LENGTH 200
#define UG_CONDITION_IMAGE_VIDEO "(MEDIA_TYPE=0 OR MEDIA_TYPE=1)"

typedef struct __ug_filter_s ug_filter_s;
struct __ug_filter_s {
	char *cond;                              /*set media type or favorite type, or other query statement*/
	media_content_collation_e collate_type;  /*collate type*/
	media_content_order_e sort_type;         /*sort type*/
	char *sort_keyword;                      /*sort keyword*/
	int offset;                              /*offset*/
	int count;                               /*count*/
	bool with_meta;                          /*whether get image or video info*/
};

typedef struct __ug_transfer_data_s ug_transfer_data_s;

struct __ug_transfer_data_s {
	const char *file_path;
	char *thumbnail_path;
	media_info_h *media;
};


struct _ug_ftype_by_mime {
	const char *mime;
	mf_ug_fs_file_type ftype;
};

static struct _ug_ftype_by_mime mime_type[] = {
	{"image/png", UG_FILE_TYPE_IMAGE},
	{"image/jpeg", UG_FILE_TYPE_IMAGE},
	{"image/gif", UG_FILE_TYPE_IMAGE},
	{"image/bmp", UG_FILE_TYPE_IMAGE},
	{"image/vnd.wap.wbmp", UG_FILE_TYPE_IMAGE},

	{"video/x-msvideo", UG_FILE_TYPE_VIDEO},
	{"video/mp4", UG_FILE_TYPE_VIDEO},
	{"video/3gpp", UG_FILE_TYPE_VIDEO},
	{"video/x-ms-asf", UG_FILE_TYPE_VIDEO},
	{"video/x-ms-wmv", UG_FILE_TYPE_VIDEO},
	{"video/x-matroska", UG_FILE_TYPE_VIDEO},

	{"audio/mpeg", UG_FILE_TYPE_MUSIC},
	{"audio/x-wav", UG_FILE_TYPE_MUSIC},
	{"application/x-smaf", UG_FILE_TYPE_MUSIC},
	{"audio/mxmf", UG_FILE_TYPE_MUSIC},
	{"audio/midi", UG_FILE_TYPE_MUSIC},
	{"audio/x-xmf", UG_FILE_TYPE_MUSIC},
	{"audio/x-ms-wma", UG_FILE_TYPE_MUSIC},
	{"audio/aac", UG_FILE_TYPE_MUSIC},
	{"audio/ac3", UG_FILE_TYPE_MUSIC},
	{"audio/ogg", UG_FILE_TYPE_MUSIC},
	{"audio/vorbis", UG_FILE_TYPE_MUSIC},
	{"audio/imelody", UG_FILE_TYPE_MUSIC},
	{"audio/iMelody", UG_FILE_TYPE_MUSIC},
	{"audio/x-rmf", UG_FILE_TYPE_MUSIC},
	{"application/vnd.smaf", UG_FILE_TYPE_MUSIC},
	{"audio/mobile-xmf", UG_FILE_TYPE_MUSIC},
	{"audio/mid", UG_FILE_TYPE_MUSIC},
	{"audio/vnd.ms-playready.media.pya", UG_FILE_TYPE_MUSIC},
	{"audio/imy", UG_FILE_TYPE_MUSIC},
	{"audio/m4a", UG_FILE_TYPE_MUSIC},
	{"audio/melody", UG_FILE_TYPE_MUSIC},
	{"audio/mmf", UG_FILE_TYPE_MUSIC},
	{"audio/mp3", UG_FILE_TYPE_MUSIC},
	{"audio/mp4", UG_FILE_TYPE_MUSIC},
	{"audio/MP4A-LATM", UG_FILE_TYPE_MUSIC},
	{"audio/mpeg3", UG_FILE_TYPE_MUSIC},
	{"audio/mpeg4", UG_FILE_TYPE_MUSIC},
	{"audio/mpg", UG_FILE_TYPE_MUSIC},
	{"audio/mpg3", UG_FILE_TYPE_MUSIC},
	{"audio/smaf", UG_FILE_TYPE_MUSIC},
	{"audio/sp-midi", UG_FILE_TYPE_MUSIC},
	{"audio/wav", UG_FILE_TYPE_MUSIC},
	{"audio/wave", UG_FILE_TYPE_MUSIC},
	{"audio/wma", UG_FILE_TYPE_MUSIC},
	{"audio/xmf", UG_FILE_TYPE_MUSIC},
	{"audio/x-mid", UG_FILE_TYPE_MUSIC},
	{"audio/x-midi", UG_FILE_TYPE_MUSIC},
	{"audio/x-mp3", UG_FILE_TYPE_MUSIC},
	{"audio/-mpeg", UG_FILE_TYPE_MUSIC},
	{"audio/x-mpeg", UG_FILE_TYPE_MUSIC},
	{"audio/x-mpegaudio", UG_FILE_TYPE_MUSIC},
	{"audio/x-mpg", UG_FILE_TYPE_MUSIC},
	{"audio/x-ms-asf", UG_FILE_TYPE_MUSIC},
	{"audio/x-wave", UG_FILE_TYPE_MUSIC},
	{"audio/x-vorbis+ogg", UG_FILE_TYPE_MUSIC},
	{"application/pdf", UG_FILE_TYPE_PDF},

	{"application/msword", UG_FILE_TYPE_DOC},
	{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", UG_FILE_TYPE_DOC},

	{"application/vnd.ms-powerpoint", UG_FILE_TYPE_PPT},
	{"application/vnd.openxmlformats-officedocument.presentationml.presentation", UG_FILE_TYPE_PPT},

	{"application/vnd.ms-excel", UG_FILE_TYPE_EXCEL},
	{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", UG_FILE_TYPE_EXCEL},

	{"audio/AMR", UG_FILE_TYPE_VOICE},
	{"audio/AMR-WB", UG_FILE_TYPE_VOICE},
	{"audio/amr", UG_FILE_TYPE_VOICE},
	{"audio/amr-wb", UG_FILE_TYPE_VOICE},
	{"audio/x-amr", UG_FILE_TYPE_VOICE},

	{"text/html", UG_FILE_TYPE_HTML},

	{"application/x-shockwave-flash", UG_FILE_TYPE_FLASH},
	{"video/x-flv", UG_FILE_TYPE_FLASH},

	{"text/plain", UG_FILE_TYPE_GUL},

	{"text/x-opml+xml", UG_FILE_TYPE_RSS},

	{"text/vnd.sun.j2me.app-descriptor", UG_FILE_TYPE_JAVA},
	{"application/x-java-archive", UG_FILE_TYPE_JAVA},
	{"application/snb", UG_FILE_TYPE_SNB},
	{"application/x-hwp", UG_FILE_TYPE_HWP},
	{"application/vnd.tizen.package", UG_FILE_TYPE_TPK},

	{NULL, UG_FILE_TYPE_ETC},
};

static char *icon_array[UG_FILE_TYPE_MAX] = {
	[UG_FILE_TYPE_DIR] = UG_ICON_FOLDER,
	[UG_FILE_TYPE_IMAGE] = UG_ICON_IMAGE,
	[UG_FILE_TYPE_VIDEO] = UG_ICON_VIDEO,
	[UG_FILE_TYPE_MUSIC] = UG_ICON_MUSIC,
	[UG_FILE_TYPE_SOUND] = UG_ICON_MUSIC,
	[UG_FILE_TYPE_PDF] = UG_ICON_PDF,
	[UG_FILE_TYPE_DOC] = UG_ICON_DOC,
	[UG_FILE_TYPE_PPT] = UG_ICON_PPT,
	[UG_FILE_TYPE_EXCEL] = UG_ICON_EXCEL,
	[UG_FILE_TYPE_VOICE] = UG_ICON_MUSIC,
	[UG_FILE_TYPE_HTML] = UG_ICON_HTML,
	[UG_FILE_TYPE_FLASH] = UG_ICON_FLASH,
	[UG_FILE_TYPE_TXT] = UG_ICON_TXT,
	[UG_FILE_TYPE_VCONTACT] = UG_ICON_VCONTACT,
	[UG_FILE_TYPE_VCALENDAR] = UG_ICON_VCALENDAR,
	[UG_FILE_TYPE_VNOTE] = UG_ICON_TXT,
	[UG_FILE_TYPE_RSS] = UG_ICON_RSS,
	[UG_FILE_TYPE_JAVA] = UG_ICON_JAVA,
	[UG_FILE_TYPE_TPK] = UG_ICON_TPK,
	[UG_FILE_TYPE_HWP] = UG_ICON_HWP,
	[UG_FILE_TYPE_SNB] = UG_ICON_SNB,
	[UG_FILE_TYPE_GUL] = UG_ICON_GUL,
};


int mf_ug_file_attr_media_has_video(const char *filename)
{
	UG_TRACE_BEGIN;
	metadata_extractor_h handle = NULL;

	if (!filename) {
		goto CATCH_ERROR;
	}
	SECURE_DEBUG("filename is [%s]", filename);
	int ret = 0;

	ret = metadata_extractor_create(&handle);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		ug_error("metadata_extractor_create().. %d", ret);
		goto CATCH_ERROR;
	}

	ret = metadata_extractor_set_path(handle, filename);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		ug_error("metadata_extractor_set_path().. %d", ret);
		goto CATCH_ERROR;
	}

	char *value = NULL;

	ret = metadata_extractor_get_metadata(handle, METADATA_HAS_VIDEO, &value);
	if (ret == METADATA_EXTRACTOR_ERROR_NONE && value) {
		if (g_strcmp0(value, "1") == 0) {
			ug_error("ret is [%d] value is [%s]", ret, "1");
			if (handle) {
				metadata_extractor_destroy(handle);
			}

			UG_SAFE_FREE_CHAR(value);
			UG_TRACE_END;
			return 1;
		}
	}
	ug_error("ret is [%d] value is [%s]", ret, value);
	UG_SAFE_FREE_CHAR(value);

	if (handle) {
		metadata_extractor_destroy(handle);
	}

	UG_TRACE_END;
	return 0;

CATCH_ERROR:
	if (handle) {
		metadata_extractor_destroy(handle);
	}

	UG_TRACE_END;
	return 0;
}


/*********************
**Function name:	__mf_ug_file_attr_get_category_by_file_ext
**Parameter:		const char* file_ext
**Return value:		mf_ug_fs_file_type
**
**Action:
**	Get file category by extention
**
*********************/
static mf_ug_fs_file_type __mf_ug_file_attr_get_category_by_file_ext(const char *file_ext, const char *fullpath)
{
	int i = 0;

	if (file_ext == NULL) {
		return UG_FILE_TYPE_ETC;
	}

	if (file_ext[0] == '.') {
		i = 1;
	}

	switch (file_ext[i]) {
	case 'a':
	case 'A':
		if (strcasecmp("ASF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("AMR", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VOICE;
		}
		if (strcasecmp("AWB", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VOICE;
		}
		if (strcasecmp("AAC", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("AVI", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("AAC", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}

		break;
	case 'b':
	case 'B':
		if (strcasecmp("BMP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		break;
	case 'd':
	case 'D':
		if (strcasecmp("DOC", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_DOC;
		}
		if (strcasecmp("DOCX", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_DOC;
		}
		if (strcasecmp("DIVX", &file_ext[i]) == 0) {
			{
				return UG_FILE_TYPE_VIDEO;
			}
		}
		break;
	case 'f':
	case 'F':
		if (strcasecmp("FLAC", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("FLV", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		break;
	case 'g':
	case 'G':
		if (strcasecmp("GIF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("G72", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("GUL", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_GUL;
		}
		break;
	case 'h':
	case 'H':
		if (strcasecmp("H263", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("HTML", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_HTML;
		}
		if (strcasecmp("HTM", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_HTML;
		}
		if (strcasecmp("HWP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_HWP;
		}
		break;
	case 'i':
	case 'I':
		if (strcasecmp("IMY", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("IPK", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_APP;
		}
		if (strcasecmp("isma", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("ismv", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		break;
	case 'j':
	case 'J':
		if (strcasecmp("JAD", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_JAVA;
		}
		if (strcasecmp("JAR", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_JAVA;
		}

		if (strcasecmp("JPG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("JPEG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("JPE", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		break;
	case 'm':
	case 'M':
		if (strcasecmp("MMF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("MP3", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("MID", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("MIDI", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("MP4", &file_ext[i]) == 0) {
			if (mf_ug_file_attr_media_has_video(fullpath)) {
				return UG_FILE_TYPE_VIDEO;
			}
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("MPG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("MPEG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("M4A", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("M3G", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_FLASH;
		}
		if (strcasecmp("MXMF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("MKV", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("MKA", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		break;
	case 'o':
	case 'O':
		if (strcasecmp("opml", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_RSS;
		}
		if (strcasecmp("ogg", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		break;
	case 'p':
	case 'P':
		if (strcasecmp("PNG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("PJPEG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("PDF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_PDF;
		}
		if (strcasecmp("PPT", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_PPT;
		}
		if (strcasecmp("PPTX", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_PPT;
		}
		if (strcasecmp("PEM", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_CERTIFICATION;
		}
		break;
	case 'r':
	case 'R':
		break;
	case 's':
	case 'S':
		if (strcasecmp("SDP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("SPM", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("SMP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("SPF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("SWF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_FLASH;
		}
		if (strcasecmp("SCN", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MOVIE_MAKER;
		}
		if (strcasecmp("SVG", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SVG;
		}
		if (strcasecmp("SVGZ", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SVG;
		}
		if (strcasecmp("SNB", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SNB;
		}
		break;
	case 't':
	case 'T':
		if (strcasecmp("TXT", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_TXT;
		}
		if (strcasecmp("THM", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_THEME;
		}
		if (strcasecmp("TPK", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_TPK;
		}
		break;
	case 'v':
	case 'V':
		if (strcasecmp("VCF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VCONTACT;
		}
		if (strcasecmp("VCS", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VCALENDAR;
		}
		if (strcasecmp("VNT", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VNOTE;
		}
		if (strcasecmp("VBM", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VBOOKMARK;
		}
		break;
	case 'w':
	case 'W':
		if (strcasecmp("WAV", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("WBMP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_IMAGE;
		}
		if (strcasecmp("WGT", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_WGT;
		}
		if (strcasecmp("WMA", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_MUSIC;
		}
		if (strcasecmp("WMV", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		break;
	case 'x':
	case 'X':
		if (strcasecmp("XLS", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_EXCEL;
		}
		if (strcasecmp("XLSX", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_EXCEL;
		}
		if (strcasecmp("XMF", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_SOUND;
		}
		if (strcasecmp("XHTML", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_HTML;
		}
		break;
	case '3':
		if (strcasecmp("3GP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("3GPP", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		if (strcasecmp("3G2", &file_ext[i]) == 0) {
			return UG_FILE_TYPE_VIDEO;
		}
		break;
	}

	return UG_FILE_TYPE_ETC;
}

/*********************
**Function name:	mf_ug_file_attr_is_valid_name
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	-0x14	if the name is invalid
**	0		if the name is valid
**
**Action:
**	check if the name is valid by file name
**
*********************/
int mf_ug_file_attr_is_valid_name(const char *filename)
{
	char *pattern;
	int ret, z, cflags = 0;
	char ebuf[128];
	regex_t reg;
	regmatch_t pm[1];
	const size_t nmatch = 1;
	/*/ToDo: ignore the file star with . */
	if (strncmp(filename, ".", 1) == 0) {
		return MYFILE_ERR_INVALID_FILE_NAME;
	}

	pattern = MYFILE_NAME_PATTERN;
	z = regcomp(&reg, pattern, cflags);

	if (z != 0) {
		regerror(z, &reg, ebuf, sizeof(ebuf));
		fprintf(stderr, "%s: pattern '%s' \n", ebuf, pattern);
		return MYFILE_ERR_INVALID_FILE_NAME;
	}

	z = regexec(&reg, filename, nmatch, pm, 0);
	if (z == REG_NOMATCH) {
		ret = MYFILE_ERR_NONE;
	} else {
		ret = MYFILE_ERR_INVALID_FILE_NAME;
	}
	regfree(&reg);
	return ret;
}


/******************************
** Prototype    : __mf_ug_file_attr_default_icon_get_by_type
** Description  : Samsung
** Input        : mf_ug_fs_file_type ftype
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
char *mf_ug_file_attr_default_icon_get_by_type(mf_ug_fs_file_type ftype)
{
	char *icon_path = NULL;

	if (icon_array[ftype]) {
		icon_path = strdup(icon_array[ftype]);
	} else {
		icon_path = strdup(UG_DEFAULT_ICON);
	}

	return icon_path;
}


/******************************
** Prototype    : mf_ug_file_attr_get_parent_path
** Description  : Samsung
** Input        : const char* path
**                char* parent_path
** Output       : None
** Return Value :
** Calls        :
** Called By    :
**
**  History        :
**  1.Date         : 2010/12/10
**    Author       : Samsung
**    Modification : Created function
**
******************************/
int mf_ug_file_attr_get_parent_path(const char *path, char **parent_path)
{
	ug_mf_retvm_if(path == NULL, MYFILE_ERR_INVALID_ARG, "path is NULL");
	ug_mf_retvm_if(parent_path == NULL, MYFILE_ERR_INVALID_ARG, "parent_path is NULL");

	*parent_path = g_strdup(path);
	if (*parent_path == NULL) {
		return MYFILE_ERR_ALLOCATE_FAIL;
	}

	const char *name = NULL;
	name = mf_file_get(path);
	/*
	**	input path and parent_path are check in the caller.
	**	parent_path is full path must be like /opt/media/file.ext
	**	name is file.ext
	**	strlen(parent_path) should large than strlen(name) normally.
	**	to take exception like input path is "", we add a if condition
	*/
	if (strlen(*parent_path) > strlen(name)) {
		(*parent_path)[strlen(*parent_path) - strlen(name) - 1] = '\0';
	}

	if (strlen(*parent_path) == 0) {
		*parent_path = g_strdup("/");
	}

	return MYFILE_ERR_NONE;
}


/*********************
**Function name:	mf_get_category
**Parameter:
**	const char* filepath:	file fullpath
**	mf_ug_fs_file_type *category:	output parameter of category
**Return value:
**	error code
**
**Action:
**	Get file category by file full path
**
*********************/
int mf_ug_file_attr_get_file_category(char *filepath, mf_ug_fs_file_type * category)
{
	int i = 0;
	int flag = 0;

	if (mf_ug_file_attr_is_dir(filepath)) {
		*category = UG_FILE_TYPE_DIR;
		return MYFILE_ERR_NONE;
	}

	const char *filename = NULL;
	filename = mf_file_get(filepath);
	if(filename)
	/*/return value ceck */
	if (filename == NULL) {
		*category = UG_FILE_TYPE_NONE;
		return MYFILE_ERR_SRC_ARG_INVALID;
	}
	char file_ext[FILE_EXT_LEN_MAX + 1] = { 0 };
	/*/ToDo: error file name like the last letter is "." */
	for (i = strlen(filename); i >= 0; i--) {
		if (filename[i] == '.') {
			strncpy(file_ext, &filename[i + 1], FILE_EXT_LEN_MAX);
			flag = 1;
			break;
		}

		if (filename[i] == '/') {
			flag = 0;
			break;
		}
	}

	if (flag == 1) {
		*category = __mf_ug_file_attr_get_category_by_file_ext(file_ext, filepath);
		return MYFILE_ERR_NONE;
	} else {
		*category = UG_FILE_TYPE_NONE;
		return MYFILE_ERR_GET_CATEGORY_FAIL;
	}
}

/*********************
**Function name:	mf_ug_file_attr_get_file_stat
**Parameter:
**	const char* filename:	file name
**	ugFsNodeInfo **node:		output parameter of what we need to refine
**Return value:
**	error code
**
**Action:
**	Get file size and last modified date by file path
**
*********************/
int mf_ug_file_attr_get_file_stat(const char *filename, ugFsNodeInfo ** node)
{
	struct stat statbuf;

	ug_mf_retvm_if(filename == NULL, MYFILE_ERR_INVALID_ARG, "filename is null");
	ug_mf_retvm_if(node == NULL, MYFILE_ERR_INVALID_ARG, "node is null");

	if (stat(filename, &statbuf) == -1) {
		return MYFILE_ERR_GET_STAT_FAIL;
	}

	(*node)->size = statbuf.st_size;
	(*node)->date = statbuf.st_mtime;

	return MYFILE_ERR_NONE;
}

int mf_ug_file_attr_get_file_size(const char *filename, off_t *size)
{

	ug_mf_retvm_if(filename == NULL, MYFILE_ERR_INVALID_ARG, "filename is null");
	struct stat statbuf;
	if (stat(filename, &statbuf) == -1) {
		return MYFILE_ERR_GET_STAT_FAIL;
	}
	*size = statbuf.st_size;
	return MYFILE_ERR_NONE;

}

/*********************
**Function name:	mf_ug_file_attr_is_dir
**Parameter:
**	const char* filename:	file fullpath
**Return value:
**	if path is a directory, return 1
**	else, return 0
**
**Action:
**	check if the file path is Directory
**
*********************/
int mf_ug_file_attr_is_dir(const char *filepath)
{
	return mf_is_dir(filepath);
}

/*********************
**Function name:	mf_ug_file_attr_get_store_type_by_full
**Parameter:
**	const char* filepath:	file full path
**	mf_ug_storage_type *store_type:		output parameter of storage type
**Return value:
**	error code
**
**Action:
**	Get file storage type by file path
**
*********************/
int mf_ug_file_attr_get_store_type_by_full(const char *filepath, mf_ug_storage_type * store_type)
{
	if (filepath == NULL || store_type == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (strncmp(filepath, PHONE_FOLDER, strlen(PHONE_FOLDER)) == 0) {
		*store_type = MF_UG_PHONE;
		return MYFILE_ERR_NONE;
	} else if (strncmp(filepath, MEMORY_FOLDER, strlen(MEMORY_FOLDER)) == 0) {
		*store_type = MF_UG_MMC;
		return MYFILE_ERR_NONE;
	} else {
		*store_type = MF_UG_NONE;
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}
}

/*********************
**Function name:	mf_ug_file_attr_get_file_ext
**Parameter:
**	const char* filepath:	file full path
**	char *file_ext:			output parameter of file extension
**
**Return value:
**	error code
**
**Action:
**	get file extension by file full path
**
*********************/
int mf_ug_file_attr_get_file_ext(const char *filepath, char **file_ext)
{
	ug_mf_retvm_if(filepath == NULL, MYFILE_ERR_INVALID_FILE_NAME, "filepath is NULL");
	ug_mf_retvm_if(file_ext == NULL, MYFILE_ERR_INVALID_FILE_NAME, "file_ext is NULL");

	const char *filename = NULL;
	filename = mf_file_get(filepath);

	if (filename == NULL) {
		return MYFILE_ERR_INVALID_FILE_NAME;
	}

	char *pdot = strrchr(filename, '.');

	if (!pdot) {
		return MYFILE_ERR_EXT_GET_ERROR;
	} else if (pdot != filepath) {
		*file_ext = g_strdup(pdot + 1);
		return MYFILE_ERR_NONE;
	} else {
		return MYFILE_ERR_EXT_GET_ERROR;
	}
}

/*********************
**Function name:	mf_ug_file_attr_is_right_dir_path
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	error code
**
**Action:
**	check if the dir path is correct
**
*********************/
int mf_ug_file_attr_is_right_dir_path(const char *dir_path)
{
	int result = MYFILE_ERR_NONE;
	int length = 0;

	length = strlen(dir_path);
	if (length == 0) {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	if (dir_path[length - 1] == '/' && length > 1) {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	if (dir_path[0] != '/') {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	const char *file_name = NULL;
	file_name = mf_file_get(dir_path);
	result = mf_ug_file_attr_is_valid_name(file_name);

	if (result != MYFILE_ERR_NONE) {
		ug_mf_error("Is NOT Valid dir path name");
	}

	return result;
}

/*********************
**Function name:	mf_ug_file_attr_is_right_file_path
**Parameter:
**	const char *filename:	the file/dir name we need to check
**
**Return value:
**	error code
**
**Action:
**	check if the file path is correct
**
*********************/
int mf_ug_file_attr_is_right_file_path(const char *file_path)
{
	int result = MYFILE_ERR_NONE;

	if (strlen(file_path) == 0) {
		return MYFILE_ERR_INVALID_FILE_PATH;
	}

	if (file_path[0] != '/') {
		return MYFILE_ERR_INVALID_DIR_PATH;
	}

	const char *file_name = NULL;
	file_name = mf_file_get(file_path);
	result = mf_ug_file_attr_is_valid_name(file_name);
	if (result != MYFILE_ERR_NONE) {
		ug_mf_error("Is NOT Valid dir path name");
	}

	return result;
}

static int __mf_ug_create_filter(filter_h *filter, ug_filter_s *condition)
{
	ug_mf_retvm_if(filter == NULL, -1, "filter is NULL");
	ug_mf_retvm_if(condition == NULL, -1, "condition is NULL");

	int ret = MEDIA_CONTENT_ERROR_NONE;
	filter_h tmp_filter = NULL;
	ret = media_filter_create(&tmp_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		return ret;
	}
	if (condition->cond) {
		ret = media_filter_set_condition(tmp_filter, condition->cond,
		                                 condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ug_debug("Fail to set condition");
			goto ERROR;
		}
	}

	if (condition->sort_keyword) {
		ret = media_filter_set_order(tmp_filter, condition->sort_type,
		                             condition->sort_keyword,
		                             condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ug_debug("Fail to set order");
			goto ERROR;
		}
	}

	if (condition->offset != -1 && condition->count != -1 &&
	        condition->count > condition->offset) {
		ret = media_filter_set_offset(tmp_filter, condition->offset,
		                              condition->count);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ug_debug("Fail to set offset");
			goto ERROR;
		}
	}
	*filter = tmp_filter;
	return ret;
ERROR:
	if (tmp_filter) {
		media_filter_destroy(tmp_filter);
		tmp_filter = NULL;
	}
	return ret;
}

static int __mf_ug_destroy_filter(filter_h filter)
{
	ug_mf_retvm_if(filter == NULL, -1, "filter is NULL");
	int ret = MEDIA_CONTENT_ERROR_NONE;
	ret = media_filter_destroy(filter);

	return ret;
}

static bool __mf_ug_local_data_get_media_thumbnail_cb(media_info_h media, void *data)
{
	ug_mf_retvm_if(data == NULL, -1, "filter is NULL");
	ug_transfer_data_s *tmp_data = (ug_transfer_data_s *)data;
	media_info_clone(tmp_data->media, media);

	media_info_get_thumbnail_path(media, &(tmp_data->thumbnail_path));

	return false;
}

int static __mf_ug_local_thumbnail_get(void *data, ug_filter_s *condition)
{

	int ret = -1;
	filter_h filter = NULL;
	ret = __mf_ug_create_filter(&filter, condition);
	if (ret != 0) {
		ug_debug("Create filter failed");
		return ret;
	}


	ret = media_info_foreach_media_from_db(filter,
	                                       __mf_ug_local_data_get_media_thumbnail_cb,
	                                       data);
	if (ret != 0) {
		ug_debug("media_info_foreach_media_from_db failed: %d", ret);
	} else {
		ug_debug("media_info_foreach_media_from_db success!", ret);
	}
	__mf_ug_destroy_filter(filter);

	return ret;

}

int mf_ug_file_attr_get_thumbnail(void *data)
{
	ug_mf_retvm_if(data == NULL, -1, "data is NULL");

	ug_transfer_data_s *mp_data = (ug_transfer_data_s *)data;
	ug_filter_s filter;
	int ret = -1;

	memset(&filter, 0, sizeof(ug_filter_s));

	char *condition = NULL;
	condition = g_strdup_printf("%s and MEDIA_PATH=\"%s\"", UG_CONDITION_IMAGE_VIDEO, mp_data->file_path);
	filter.cond = condition;
	filter.collate_type = MEDIA_CONTENT_COLLATE_DEFAULT;
	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	filter.sort_keyword = MEDIA_MODIFIED_TIME;
	filter.with_meta = true;

	ret = __mf_ug_local_thumbnail_get(data, &filter);
	UG_SAFE_FREE_CHAR(condition);


	return ret;
}

int mf_ug_file_attr_get_file_icon(char *file_path, int *error_code, char **thumbnail, media_info_h *media_info)
{
	int index = 0;
	char *icon_path = NULL;
	mf_ug_fs_file_type ftype = UG_FILE_TYPE_NONE;
	int thumbnail_type = MF_UG_THUMBNAIL_TYPE_DEFAULT;
	char *mime = NULL;
	int retcode = -1;

	ug_mf_retvm_if(file_path == NULL, MF_UG_THUMBNAIL_TYPE_DEFAULT, "file_path is NULL");

	int ret = mf_ug_file_attr_get_file_category(file_path, &ftype);
	if (ret != MYFILE_ERR_NONE || ftype == UG_FILE_TYPE_NONE || ftype == UG_FILE_TYPE_ETC) {
		retcode = mime_type_get_mime_type(file_path, &mime);
		if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
			ug_debug("Fail to get mime type, set etc icon");
			return thumbnail_type;
		}

		ug_debug("mime is [%s]", mime);
		for (index = 0; mime_type[index].mime; index++) {
			if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
				ftype = mime_type[index].ftype;
				break;
			}
		}
	}

	UG_SAFE_FREE_CHAR(mime);

	switch (ftype) {
	case UG_FILE_TYPE_IMAGE:
	case UG_FILE_TYPE_VIDEO: {
		int err = 0;
		ug_transfer_data_s tmp_data;
		memset(&tmp_data, 0x00, sizeof(ug_transfer_data_s));
		tmp_data.file_path = file_path;
		tmp_data.media = media_info;
		err = mf_ug_file_attr_get_thumbnail(&tmp_data);
		if (err == 0) {
			icon_path = g_strdup(tmp_data.thumbnail_path);
			thumbnail_type = MF_UG_THUMBNAIL_TYPE_THUMBNAIL;
		} else {
			icon_path = NULL;
			if (error_code) {
				*error_code = err;
			}
		}
	}
	break;
	default:
		icon_path = mf_ug_file_attr_default_icon_get_by_type(ftype);
		thumbnail_type = MF_UG_THUMBNAIL_TYPE_DEFAULT;
		break;
	}

	*thumbnail = icon_path;
	return thumbnail_type;
}

static int mf_ug_file_attr_get_path_level(const char *fullpath, int *level)
{
	if (fullpath == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (mf_ug_file_attr_is_right_dir_path(fullpath) != 0) {
		return MYFILE_ERR_INVALID_PATH;
	}

	mf_ug_storage_type storage_t = 0;
	int start_level = 0;
	int error_code = mf_ug_file_attr_get_store_type_by_full(fullpath, &storage_t);
	if (error_code != 0) {
		return error_code;
	}

	if (storage_t == MF_UG_PHONE) {
		start_level = MF_UG_PHONE_DEFAULT_LEVEL;
	} else if (storage_t == MF_UG_MMC) {
		start_level = MF_UG_MMC_DEFAULT_LEVEL;
	}

	char *temp = strdup(fullpath);
	if (temp == NULL) {
		return MYFILE_ERR_UNKNOW_ERROR;
	}

	int count = 0;

	gchar **result = NULL;
	gchar **params = NULL;
	result = g_strsplit(temp, "/", 0);

	if (result == NULL) {
		free(temp);
		temp = NULL;
		return MYFILE_ERR_UNKNOW_ERROR;
	}

	for (params = result; *params; params++) {
		count++;
	}

	g_strfreev(result);
	*level = count - start_level - 1;
	free(temp);
	return MYFILE_ERR_NONE;

}

int mf_ug_file_attr_is_in_system_folder(char *fullpath, int level, bool * result)
{
	if (fullpath == NULL) {
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	mf_ug_storage_type storage_t = 0;
	int error_code = mf_ug_file_attr_get_store_type_by_full(fullpath, &storage_t);
	if (error_code != 0) {
		return error_code;
	}

	const char *name = NULL;
	name = mf_file_get(fullpath);
	char *parent_path = malloc(MYFILE_DIR_PATH_LEN_MAX + 1);

	if (parent_path == NULL) {
		return MYFILE_ERR_ALLOCATE_FAIL;
	}
	memset(parent_path, 0, MYFILE_DIR_PATH_LEN_MAX + 1);
	error_code = mf_ug_file_attr_get_parent_path(fullpath, &parent_path);

	if (error_code != 0) {

		free(parent_path);
		parent_path = NULL;
		return error_code;
	}

	if (storage_t == MF_UG_PHONE || storage_t == MF_UG_MMC) {
		if (level == 1) {
			if ((strlen(name) == strlen(IMAGE_AND_VIDEO)) && strcmp(name, IMAGE_AND_VIDEO) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(SOUND_AND_MUSIC)) && strcmp(name, SOUND_AND_MUSIC) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(DOWNLOADS)) && strcmp(name, DOWNLOADS) == 0) {
				*result = true;
			} else if ((strlen(name) == strlen(CAMERA_SHOTS)) && strcmp(name, CAMERA_SHOTS) == 0) {
				*result = true;
			} else {
				*result = false;
			}
		} else if (level == 2) {
			const char *parent_name = NULL;
			parent_name = mf_file_get(parent_path);
			if (storage_t == MF_UG_PHONE && (strlen(parent_name) == strlen(DOWNLOADS)) && strcmp(parent_name, DOWNLOADS) == 0) {
				if ((strlen(name) == strlen(OTHERS)) && !strcmp(OTHERS, name)) {
					*result = true;
				} else {
					*result = false;
				}
			} else if ((strlen(parent_name) == strlen(SOUND_AND_MUSIC)) && strcmp(parent_name, SOUND_AND_MUSIC) == 0) {
				if ((strlen(name) == strlen(FM_RADIO)) && !strcmp(FM_RADIO, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(MUSIC)) && !strcmp(MUSIC, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(RINGTONES)) && !strcmp(RINGTONES, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(ALERTS)) && !strcmp(ALERTS, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(VOICE_RECORDER)) && !strcmp(VOICE_RECORDER, name)) {
					*result = true;
				} else {
					*result = false;
				}
			} else if ((strlen(parent_name) == strlen(IMAGE_AND_VIDEO)) && strcmp(parent_name, IMAGE_AND_VIDEO) == 0) {
				if ((strlen(name) == strlen(WALLPAPER)) && !strcmp(WALLPAPER, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(MY_PHOTO_CLIPS)) && !strcmp(MY_PHOTO_CLIPS, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(MY_ALBUM)) && !strcmp(MY_ALBUM, name)) {
					*result = true;
				} else if ((strlen(name) == strlen(MY_VIDEO_CLIPS)) && !strcmp(MY_VIDEO_CLIPS, name)) {
					*result = true;
				} else {
					*result = false;
				}
			} else {
				*result = false;
			}
		} else {
			if (parent_path) {
				free(parent_path);
				parent_path = NULL;
			}
			return MYFILE_ERR_STORAGE_TYPE_ERROR;
		}
	}

	else {
		if (parent_path) {
			free(parent_path);
			parent_path = NULL;
		}
		*result = false;
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}

	if (parent_path) {
		free(parent_path);
		parent_path = NULL;
	}
	return MYFILE_ERR_NONE;
}

int mf_ug_file_attr_is_system_dir(char *fullpath, bool * result)
{
	if (fullpath == NULL) {
		ug_debug("source argument invalid");
		return MYFILE_ERR_SRC_ARG_INVALID;
	}

	if (mf_ug_file_attr_is_dir(fullpath) == 0) {
		ug_debug("source is not exist");
		return MYFILE_ERR_SRC_NOT_EXIST;
	}

	int level = 0;
	int error_code = 0;

	error_code = mf_ug_file_attr_get_path_level(fullpath, &level);
	if (error_code != 0) {
		ug_debug("Fail to get path level");
		return error_code;
	}

	if (level >= 3 || level <= 0) {
		*result = false;
		ug_debug("Path Level is wrong");
		return MYFILE_ERR_NONE;
	}
	error_code = mf_ug_file_attr_is_in_system_folder(fullpath, level, result);

	if (error_code != 0) {
		ug_debug("Fail .. is in system folder err :: %d", error_code);
		return error_code;
	}

	return MYFILE_ERR_NONE;

}

mf_ug_fs_file_type mf_ug_file_attr_get_file_type(const char *mime)
{
	int index;
	mf_ug_fs_file_type ftype = UG_FILE_TYPE_NONE;
	for (index = 0; mime_type[index].mime; index++) {
		if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
			ftype = mime_type[index].ftype;
		}
	}
	return ftype;

}

mf_ug_fs_file_type mf_ug_file_attr_get_file_type_by_mime(const char *file_path)
{
	int index;
	mf_ug_fs_file_type ftype = UG_FILE_TYPE_NONE;
	char *mime = NULL;
	int retcode = -1;

	retcode = mime_type_get_mime_type(file_path, &mime);
	if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
		ug_debug("Fail to aul_get_mime_from_file(), set etc icon");
		return ftype;
	}

	for (index = 0; mime_type[index].mime; index++) {
		if (strncmp(mime, mime_type[index].mime, strlen(mime)) == 0) {
			ftype = mime_type[index].ftype;
			UG_SAFE_FREE_CHAR(mime);
			return ftype;
		}
	}

	UG_SAFE_FREE_CHAR(mime);
	return ftype;
}

int mf_ug_file_attr_is_duplicated_name(const char *dir, const char *name)
{

	char *file_path = g_strconcat(dir, "/", name, NULL);

	if (file_path != NULL) {
		if (mf_file_exists(file_path)) {
			UG_SAFE_FREE_CHAR(file_path);
			return MYFILE_ERR_DUPLICATED_NAME;
		} else {
			UG_SAFE_FREE_CHAR(file_path);
			return MYFILE_ERR_NONE;
		}
	}

	return MYFILE_ERR_NONE;
}

int mf_ug_file_attr_get_logical_path_by_full(const char *full_path, char **path)
{
	ug_mf_retvm_if(full_path == NULL, MYFILE_ERR_INVALID_FILE_PATH, "fullpath is NULL");
	ug_mf_retvm_if(path == NULL, MYFILE_ERR_INVALID_FILE_PATH, "path is NULL");

	mf_ug_storage_type store_type = 0;
	int root_len = 0;

	mf_ug_file_attr_get_store_type_by_full(full_path, &store_type);

	*path = g_strdup(full_path);
	if (*path == NULL) {
		return MYFILE_ERR_ALLOCATE_MEMORY_FAIL;
	}

	memset(*path, 0, strlen(*path));
	switch (store_type) {
	case MF_UG_PHONE:
		root_len = strlen(PHONE_FOLDER);
		break;
	case MF_UG_MMC:
		root_len = strlen(MEMORY_FOLDER);
		break;
	default:
		return MYFILE_ERR_STORAGE_TYPE_ERROR;
	}

	/*
	**	*path has the same length with full_path
	**	strlen(*path) is 0 since the memset called
	**	we use length of full_path to reprecent the *path's
	*/
	g_strlcpy(*path, full_path + root_len, strlen(full_path));
	if (strlen(*path) == 0) {
		UG_SAFE_FREE_CHAR(*path);
		*path = g_strdup("/");
	}

	return MYFILE_ERR_NONE;
}

char *mf_ug_file_attr_sound_title_get(const char *fullpath)
{
	metadata_extractor_h metadata = NULL;
	char *title = NULL;
	int ret = metadata_extractor_create(&metadata);
	if (ret == METADATA_EXTRACTOR_ERROR_NONE && metadata) {
		ret = metadata_extractor_set_path(metadata, fullpath);
		if (ret == METADATA_EXTRACTOR_ERROR_NONE) {
			ret = metadata_extractor_get_metadata(metadata, METADATA_TITLE, &title);
		}
		metadata_extractor_destroy(metadata);
	}

	return title;
}
