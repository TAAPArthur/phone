#!/bin/sh
mqreceive phone | ttyio | mqbus-send phone-response
