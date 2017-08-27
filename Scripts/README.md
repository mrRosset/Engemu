To generate a list that map the address of the rom function with the name of the function:

1. Have 3 Folder ready:
	* A folder with a copy of all the files from the Z:\System\Libs on an n-gage  
  	* A folder with a copy of all idt files found from ida ids.
  	* A temporary folder
  	* A folder to hold the resulting mapping
1. In a terminal run `python Export_all_Dlls.py DLLFodler TempFolder`
1. In a terminal run `python Export_all_Symbols.py TempFolder IdtFolder ResultFolder`
