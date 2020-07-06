rmdir /s/q ".vs"
rmdir /s/q bin
rmdir /s/q bin-int

@for /R %%I in (*.vcxproj*) do del "%%I"
@for /R %%I in (*.sln) do del "%%I"

PAUSE