/************************ Root Required HTML Page *****************************
This file is part of the PDI stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

The `RootRequired.h` file defines the HTML content served in place of any page
reserved to the root user. A session that is not root receives this instead of
the page it asked for, so a restricted setting is neither shown nor accepted.
It carries a link back to the home page. The HTML content is stored in program
memory (PROG_RODT_ATTR) to optimize memory usage on embedded systems.

Author          : Suraj I.
Created Date    : 3rd Sep 2026
******************************************************************************/

#ifndef _WEB_SERVER_ROOT_REQUIRED_PAGE_H_
#define _WEB_SERVER_ROOT_REQUIRED_PAGE_H_

#include <interface/pdi.h>

/**
 * @brief HTML content for the "root privilege required" page.
 *
 * This static HTML content is served whenever a session that is not root
 * requests a restricted page. It states what is required and includes a
 * button linking to the home page.
 */
static const char WEB_SERVER_ROOT_REQUIRED_PAGE[] PROG_RODT_ATTR = "\
<h3>Root Privilege Required</h3>\
<div>\
Login as root user to view or change this setting.\
</div>\
<br>\
<div>\
Go to\
<a href='/'>\
<button class='btn'>\
Home\
</button>\
</a>\
</div>";

#endif
