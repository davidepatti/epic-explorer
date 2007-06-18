/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sab ago 17 14:48:41 CEST 2002
    copyright            : (C) 2002 by Davide Patti
    email                : davidepatti@email.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "user_interface.h"
#include "explorer.h"
#include "common.h"
#include <signal.h>

int main(int argc, char *argv[])
{
    string base_path = string(getenv(BASE_DIR));
    User_interface *ui = new User_interface(base_path);
    ui->interact();
    return EXIT_SUCCESS;
}
