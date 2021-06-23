@echo off
pushd %~dp0


for /r "projects\CLARINOID2\src" %%f in (*.?pp) do (
  call clang-format -i --style=file "%%f" --verbose
)

for /r "projects\BASSOONOID\src" %%f in (*.?pp) do (
  call clang-format -i --style=file "%%f" --verbose
)

for /r "src\Clarinoid" %%f in (*.?pp) do (
  call clang-format -i --style=file "%%f" --verbose
)


pause
