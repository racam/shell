cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

project(shell)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -std=gnu99")

#####
# Détection de la bibliothèque Readline
#####
find_library(READLINE_LIBRARY NAMES readline libreadline history libhistory) 


##
# Si vous utilisé plusieurs fichiers, en plus de ensishell.c, pour votre
# shell il faut les ajouter ici
##
add_executable(shell src/readcmd.c src/shell.c src/jobsList.c)
target_link_libraries(shell readline history)