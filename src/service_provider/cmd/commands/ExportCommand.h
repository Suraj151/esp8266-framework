/******************************* Export Command *******************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 5th Sep 2026
******************************************************************************/
#ifndef _EXPORT_COMMAND_H_
#define _EXPORT_COMMAND_H_

#include "CommandCommon.h"
#include <service_provider/session/Environment.h>

struct ExportCommand : public CommandBase {

	ExportCommand(){
		Clear();
		SetCommand(CMD_NAME_EXPORT);
		setAcceptArgsOptions(true);
	}

	static void RegisterCommand(){
		CommandBase::RegisterCommand(CMD_NAME_EXPORT, [](void *arg)->void *{
			return pdiutil::safe_new<ExportCommand>();
		});
	}

	const char* getUsage() const override {
		return RODT_ATTR("export <name>=<value>  set a variable for this session");
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
			m_terminal->writeln_ro(RODT_ATTR("give it a name and a value"));
			return CMD_ERROR_ARGS_MISSING;
		}

		pdiutil::string arg(cmdoptn->optionval, cmdoptn->optionvalsize);
		pdiutil::string assign = CHARPTR_WRAP(CMD_OPTION_ASSIGN_OPERATOR);

		pdiutil::string::size_type split = arg.find(assign);
		if( split == pdiutil::string::npos ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("give it a name and a value"));
			return CMD_ERROR_ARGS_MISSING;
		}

		pdiutil::string name = arg.substr(0, split);
		pdiutil::string value = arg.substr(split + assign.size());

		pdi_err_t res = Environment::set(name.c_str(), value.c_str());

		if( CMD_ERROR_PERM == res ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("the session answers for that name, it cannot be set"));
		}else if( CMD_ERROR_INVAL == res ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("a name is a letter or underscore, then letters, digits or underscores"));
		}else if( PDI_OK != res ){
			m_terminal->putln();
			m_terminal->writeln_ro(RODT_ATTR("this session holds no more variables"));
		}

		return res;
	}
};

#endif
