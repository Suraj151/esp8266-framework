/******************************** Test Command ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 6th Sep 2026
******************************************************************************/
#ifndef _TEST_COMMAND_H_
#define _TEST_COMMAND_H_

#include "CommandCommon.h"

struct TestCommand : public CommandBase {

	TestCommand(){
		Clear();
		SetCommand(CMD_NAME_TEST);
		setAcceptArgsOptions(true);
		setCmdOptionSeparator(CMD_OPTION_SEPERATOR_SPACE);
	}

	static void RegisterCommand(){
		CommandBase::RegisterCommand(CMD_NAME_TEST, [](void *arg)->void *{
			return pdiutil::safe_new<TestCommand>();
		});
	}

	const char* getUsage() const override {
		return RODT_ATTR("test <a> = != -eq -ne -lt -le -gt -ge <b> | -z -n <s> | -e -f -d <path>");
	}

#ifdef ENABLE_AUTH_SERVICE
	bool needauth() override { return true; }
#endif

	/**
	 * One argument as text, empty when the slot carries nothing.
	 */
	pdiutil::string arg(int8_t idx){
		CommandOption *opt = &m_options[idx];
		if( nullptr == opt->optionval || 0 >= opt->optionvalsize ){
			return pdiutil::string();
		}
		return pdiutil::string(opt->optionval, opt->optionvalsize);
	}

	/**
	 * Whether every character of the text reads as a decimal number, so a
	 * numeric comparison is not silently given a word.
	 */
	bool isNumber(const pdiutil::string &s){
		if( s.empty() ) return false;

		pdiutil::string::size_type at = ('-' == s[0]) ? 1 : 0;
		if( at >= s.size() ) return false;

		while( at < s.size() ){
			if( !__is_digit(s[at]) ) return false;
			at++;
		}
		return true;
	}

	pdi_err_t execute(cmd_term_inseq_t terminputaction){

#ifdef ENABLE_AUTH_SERVICE
		if( needauth() && !__auth_service.getAuthorized() ){
			return CMD_ERROR_PERM;
		}
#endif

		pdiutil::string one = arg(0);
		pdiutil::string two = arg(1);
		pdiutil::string three = arg(2);

		if( one.empty() ){
			return CMD_ERROR_ARGS_MISSING;
		}

		if( three.empty() ){

			if( two.empty() ){
				return CMD_RESULT_FALSE;
			}

			pdiutil::string zero = CHARPTR_WRAP("-z");
			pdiutil::string nonzero = CHARPTR_WRAP("-n");
			pdiutil::string exists = CHARPTR_WRAP("-e");
			pdiutil::string isfile = CHARPTR_WRAP("-f");
			pdiutil::string isdir = CHARPTR_WRAP("-d");

			if( one == zero ) return two.empty() ? PDI_OK : CMD_RESULT_FALSE;
			if( one == nonzero ) return two.empty() ? CMD_RESULT_FALSE : PDI_OK;

#ifdef ENABLE_STORAGE_SERVICE
			if( one == exists || one == isfile || one == isdir ){

				pdiutil::string path = resolveArgPathStr(two.c_str(), (int16_t)two.size());
				if( 0 == path.size() ) return CMD_RESULT_FALSE;

				if( one == isdir ){
					return __i_fs.isDirectory(path.c_str()) ? PDI_OK : CMD_RESULT_FALSE;
				}

				bool present = __i_fs.isFileExist(path.c_str()) || __i_fs.isDirectory(path.c_str());

				if( one == isfile ){
					return (present && !__i_fs.isDirectory(path.c_str())) ? PDI_OK : CMD_RESULT_FALSE;
				}

				return present ? PDI_OK : CMD_RESULT_FALSE;
			}
#endif

			return CMD_ERROR_INVAL;
		}

		pdiutil::string equal = CHARPTR_WRAP("=");
		pdiutil::string notequal = CHARPTR_WRAP("!=");

		if( two == equal ) return (one == three) ? PDI_OK : CMD_RESULT_FALSE;
		if( two == notequal ) return (one == three) ? CMD_RESULT_FALSE : PDI_OK;

		pdiutil::string numeq = CHARPTR_WRAP("-eq");
		pdiutil::string numne = CHARPTR_WRAP("-ne");
		pdiutil::string numlt = CHARPTR_WRAP("-lt");
		pdiutil::string numle = CHARPTR_WRAP("-le");
		pdiutil::string numgt = CHARPTR_WRAP("-gt");
		pdiutil::string numge = CHARPTR_WRAP("-ge");

		bool numeric = (two == numeq) || (two == numne) || (two == numlt) ||
					   (two == numle) || (two == numgt) || (two == numge);

		if( !numeric ){
			return CMD_ERROR_INVAL;
		}

		if( !isNumber(one) || !isNumber(three) ){
			return CMD_ERROR_INVAL;
		}

		int32_t left = StringToInt32(one.c_str(), (uint8_t)one.size());
		int32_t right = StringToInt32(three.c_str(), (uint8_t)three.size());

		bool answer = false;
		if( two == numeq ) answer = (left == right);
		else if( two == numne ) answer = (left != right);
		else if( two == numlt ) answer = (left < right);
		else if( two == numle ) answer = (left <= right);
		else if( two == numgt ) answer = (left > right);
		else answer = (left >= right);

		return answer ? PDI_OK : CMD_RESULT_FALSE;
	}
};

#endif
