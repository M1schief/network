@echo off
:: appname ������
:: inputname �����ļ���
:: outputname ����ļ���
:: resultname �������̨����ض����ļ���

set appname="..\Debug\rdt.exe"
set inputname="input.txt"
set GBNoutputname="GBNoutput.txt"
set SRoutputname="SRoutput.txt"
set TCPoutputname="TCPoutput.txt"
set resultname="result.txt"

for /l %%i in (1,1,10) do (
    echo Test %appname% %%i:
    %appname% > %resultname% 2>&1
    fc /N %inputname% %GBNoutputname%
    fc /N %inputname% %SRoutputname%
    fc /N %inputname% %TCPoutputname%
)
pause