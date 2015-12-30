# MyReaperPlugin
Example C++ reaper plugin project for Xcode and Visual Studio. This has actually expanded in scope quite a bit. It now incorporates a fairly usable GUI framework that encapsulates some win32 "common controls" as C++ classes. There is also a way to create custom controls/widgets. Recently an abstraction was added that allows the extension plugin to use the Reaper audio preview system relatively conveniently. (Useful for stuff like wave editors etc...) Also, a set of functions is exported from the plugin, that may be useful for use with ReaScript. 

***Special thanks to:***
* *COCKOS for existing*
* *SWS for trailblazing*
* *Xenakios for all his persistence in helping me and the community in getting example Xcode and Visual Studio projects. The MyReaperPlugin example project is 99% Xenakios original code.*
* *Breeder for helping me get a virtual mac running on my windows machine and answering all my REAPER API questions.*
* *Etienne Laurin for answering a million C++ questions.*
