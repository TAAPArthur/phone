#!/bin/sh
mqreceive phone | (ttyio | mqbuse-send phone-response; echo | mqsend phone)
