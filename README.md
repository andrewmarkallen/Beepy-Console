
Project {#Beepy Console}
===================


#Overview

This project implements a simple console logger and waveform viewer on an STM32F429 Discovery Board. Another board (or any device) can connect to it via a serial interface and using the simple syntax described in the source write either to a console or to one of two waveform buffers to display a waveform. The project was created to make debugging on another STM32 board (without an LCD screen) easier. The project is provided in the form of a CooCox project, and relies on the TM STM libraries written by Tilen Majerles (http://stm32f4-discovery.com).