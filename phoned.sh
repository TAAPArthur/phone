#!/bin/sh
mqreceive phone | ttyio | mqbuse-send phone-response
