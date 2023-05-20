# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(main_unit)

target_sources(app PRIVATE
	src/main.c
	src/coap.c
	src/gnss.c
	src/led_button.c
	src/lte.c
	src/sensors.c
	)
