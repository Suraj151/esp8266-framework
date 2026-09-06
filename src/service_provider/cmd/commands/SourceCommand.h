/******************************* Source Command *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 5th Sep 2026
******************************************************************************/
#ifndef _SOURCE_COMMAND_H_
#define _SOURCE_COMMAND_H_

#include "CommandCommon.h"
#include <service_provider/cmd/ScriptRunner.h>

struct SourceCommand : public CommandBase {

	SourceCommand(){
		Clear();
		SetCommand(CMD_NAME_SOURCE);
		setAcceptArgsOptions(true);
	}

	static void RegisterCommand(){
		CommandBase::RegisterCommand(CMD_NAME_SOURCE, [](void *arg)->void *{
			return pdiutil::safe_new<SourceCommand>();
		});
	}

	const char* getUsage() const override {
		return RODT_ATTR("source <file>  run each line of the file in this session");
	}

#ifdef ENABLE_AUTH_SERVICE
	bool needauth() override { return true; }
#endif

	pdi_err_t execute(cmd_term_inseq_t terminputaction){

#ifdef ENABLE_AUTH_SERVICE
		if( needauth() && !__auth_service.getAuthorized() ){
			return CMD_ERROR_PERM;
		}
#endif

		if( nullptr == m_terminal ){
			return CMD_ERROR_NOTTY;
		}

		CommandOption *cmdoptn = &m_options[0];
		if( nullptr == cmdoptn->optionval || 0 >= cmdoptn->optionvalsize ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("give it a file to run"));
			return CMD_ERROR_ARGS_MISSING;
		}

		pdiutil::string arg(cmdoptn->optionval, cmdoptn->optionvalsize);
		pdiutil::string path = resolveArgPathStr(arg.c_str(), (int16_t)arg.size());

		if( 0 == path.size() ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("no such file"));
			return CMD_ERROR_FAILED;
		}

		pdi_err_t res = ScriptRunner::run(path.c_str(), false);

		if( CMD_ERROR_FAILED == res && !ScriptRunner::pending() ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("cannot run that file"));
		}

		return res;
	}
};

#endif
