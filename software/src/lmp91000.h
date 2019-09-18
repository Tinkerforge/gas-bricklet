/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * lmp91000.h: Driver for LMP91000 Potentiostat
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef LMP91000_H
#define LMP91000_H

void lmp91000_task_tick(void);
void lmp91000_task_init(void);

#define LMP91000_REG_STATUS    0x00
#define LMP91000_REG_LOCK      0x01
#define LMP91000_REG_TIACN     0x10
#define LMP91000_REG_REFCN     0x11
#define LMP91000_REG_MODECN    0x12

#endif