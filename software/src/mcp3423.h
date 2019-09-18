/* gas-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * mcp3423.h: Driver for MCP3423 ADC
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

#ifndef MCP3423_H
#define MCP3423_H

void mcp3423_task_tick(void);
void mcp3423_task_init(void);

#define MCP3423_CONF_MSK_Gx1 0x00
#define MCP3423_CONF_MSK_Gx2 0x01
#define MCP3423_CONF_MSK_Gx4 0x02
#define MCP3423_CONF_MSK_Gx8 0x03
#define MCP3423_CONF_MSK_CH0 0x00
#define MCP3423_CONF_MSK_CH1 0x20
#define MCP3423_CONF_MSK_RDY0 0x00
#define MCP3423_CONF_MSK_RDY1 0x80
#define MCP3423_CONF_MSK_SPS4 0x0C
#define MCP3423_CONF_MSK_SPS15 0x08
#define MCP3423_CONF_MSK_SPS60 0x04
#define MCP3423_CONF_MSK_SPS240 0x00
#define MCP3423_CONF_MSK_MODE_CONT 0x10
#define MCP3423_CONF_MSK_MODE_ONE_SHOT 0x00

#endif