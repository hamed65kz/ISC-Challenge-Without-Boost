#!/bin/bash

#Attention : This file should have line ending in unix format otherwize throw error in unix. save file in LF line ending, not CRLF. it is configurable in vscode

#This script initializes VCPKG in user home folder and installs packages listed in the dependencies.txt file.
#git should be installed.
#this script should run in git bash in Windows 

#remain issues:
#write bachrc $HOME instead its value.
#python/python3

#This function Detect OS and fills the OS_NAME variable, OS_NAME will be used in other function
detect_os(){
    OS_NAME='Unknown'
    # Detect the operating system
    if [ "$(uname)" == "Darwin" ]; then
        OS_NAME="macOS"
    elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
        OS_NAME="Linux"
    elif [ "$(expr substr $(uname -s) 1 5)" == "MINGW" ]; then
        OS_NAME="Windows"
    else
        my_echo_red "Unsupported operating system."
        my_echo_red "VCPKG configuration terminate with error."
        exit 1
    fi
    
    my_echo_green "Detected Platform is : $OS_NAME"

}

#git will check if the current user is the owner of the git repository folder
#may be user folder was created and owned by the System user, not the current user.
#The workaround provided by git is to add the current folder to the safe. directory global variable, so that git will regard the folder as safe.
adjust_git_safe_dir(){
    command="git config --global --add safe.directory $HOME/vcpkg"
    eval "$command"
    check_last_command_exitcode_for_warning "$command" "vcpkg added to git safe directories. "
    #my_echo_green "$HOME/vcpkg added to git safe directories."
}

# Function to clone and bootstrap vcpkg
init_vcpkg() {
    my_echo_green "cloning vcpkg repo in $HOME/vcpkg ..."
    script_path="$(pwd)"
    if [ ! -d "$HOME/vcpkg" ]; then

        command="git clone https://github.com/microsoft/vcpkg.git $HOME/vcpkg"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "vcpkg is inited in $HOME/vcpkg"

        cd $HOME/vcpkg 
        bootstrap_vcpkg              
    else      
        cd $HOME/vcpkg       
        #command="git pull"
        #eval "$command"
        #check_last_command_exitcode_for_warning "$command" "vcpkg already exist."
        my_echo_green "vcpkg already exist."

        if [ ! -f "$HOME/vcpkg/vcpkg" ]; then
            my_echo_yellow "vcpkg didnt bootstrapped. bootstrap script will be called"
            if [ -f "$HOME/vcpkg/bootstrap-vcpkg.sh" ]; then
                bootstrap_vcpkg
            else
                my_echo_red "bootstrap-vcpkg.sh doesnt exist. vcpkg didnt cloned properly."
                my_echo_red "Failed. VCPKG configuration terminate with error."
        exit 1
            fi
        fi

    fi
    
    # set back current directory to script path
    cd "$script_path"
}

#This function installs the bootstrap needed packages and Run it
bootstrap_vcpkg(){
    install_linux_package
    my_echo_green "bootstrapping vcpkg ..."
    if [ "$OS_NAME" == "Windows" ]; then
        command="./bootstrap-vcpkg.bat"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "VCPKG successfully bootstrapped."
    else
        command="./bootstrap-vcpkg.sh"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "VCPKG successfully bootstrapped."
    fi
}

get_triplet_tail(){
    
    if [ "$OS_NAME" == "Windows" ]; then
        if [ "$1" == "static" ] ; then
            local tail="-static"
            echo $tail    
        elif [ "$1" == "non-static" ] || [ "$1" == "default" ] || [ "$1" == "" ]; then
            local tail=""
            echo $tail    
        else
            my_echo_red "Unsupported key in Dependencies.txt. supported key = [static/non-static/default]"
            my_echo_red "Failed. VCPKG configuration terminate with error."
            exit 1
        fi             
    elif [ "$OS_NAME" == "Linux" ] || [ "$OS_NAME" == "macOS" ]; then
        if [ "$1" == "static" ]  || [ "$1" == "default" ] || [ "$1" == "" ]; then
            local tail=""
            echo $tail           
        elif [ "$1" == "non-static" ]; then
            local tail="-dynamic"
            echo $tail
        else
            my_echo_red "Unsupported key in Dependencies.txt. supported key = [static/non-static/default]"
            my_echo_red "Failed. VCPKG configuration terminate with error."
            exit 1                                   
        fi                     
    fi
}
#This Function is to install dependencies listed in the dependencies.txt file.
#file format is [package_name:static/non-static]
install_dependencies() {
    my_echo_green "Install dependencies listed in $dependencies_file_path"
    script_path="$(pwd)"
    while IFS=$':' read -r package static_arch
        do
            #remove CR and LF from end of variables
            static_arch=$(echo "$static_arch" | tr -d '\r')
            package=$(echo "$package" | tr -d '\r')

            # Trim leading whitespace
            package=${package##[[:space:]]}
            static_arch=${static_arch##[[:space:]]}

            # Trim trailing whitespace
            package=${package%%[[:space:]]}
            static_arch=${static_arch%%[[:space:]]}

            #len=$(expr length "$package")
            len=${#package}
            if [ $len = 0 ]; then
                #skip empty lines
                continue
            fi

            first_char=${package:0:1}
            if [ "$first_char" = "#" ]; then
                #skip comment lines
                continue
            fi

            triplet_tail=$(get_triplet_tail $static_arch)

            if [ "$OS_NAME" == "Windows" ]; then
                    triplet="x64-windows${triplet_tail}"
                    my_echo_green "Installing package $package : $triplet ..."           
                    install_vcpkg_package $package $triplet

            elif [ "$OS_NAME" == "Linux" ]; then
                    triplet="x64-linux${triplet_tail}"
                    my_echo_green "Installing package $package : $triplet ..."           
                    install_vcpkg_package $package $triplet     

            elif [ "$OS_NAME" == "macOS" ]; then

                    triplet="x64-osx${triplet_tail}"
                    my_echo_green "Installing package $package : $triplet ..."           
                    install_vcpkg_package $package $triplet

                    triplet="arm64-osx${triplet_tail}"
                    my_echo_green "Installing package $package : $triplet ..."           
                    install_vcpkg_package $package $triplet
            fi
        done < "$dependencies_file_path"
    create_universal_bin_on_macos $triplet_tail
}

#Create VCPKG_ROOT variable and add it to the PATH
#the console needs to restart in order to see this variable and use it
#To avoid restarting, we don't use them here and use absolute paths.
add_to_env(){
    if [ "$OS_NAME" == "Windows" ]; then
        command="SETX "VCPKG_ROOT" "$HOME/vcpkg""
        eval "$command"
        check_last_command_exitcode_for_error "$command" "VCPKG_ROOT added to PATH."
        SETX "PATH" "%VCPKG_ROOT%;%PATH%"
    elif [ "$OS_NAME" == "Linux" ]; then
        #Warning : each time running create a new lines in bashrc, after multiple run it create same var multiple times
        
        #add to ~/.bashrc if not added previously
        command="grep -qxF 'export VCPKG_ROOT="$HOME/vcpkg"' ~/.bashrc || echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc"
        #command="echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "VCPKG_ROOT added to PATH."

        command="grep -qxF 'export PATH=$PATH:"$HOME/vcpkg"' ~/.bashrc || echo 'export PATH=$PATH:"$HOME/vcpkg"' >> ~/.bashrc"
        eval "$command"
        #echo 'export PATH=$PATH:"$HOME/vcpkg"' >> ~/.bashrc

    elif [ "$OS_NAME" == "macOS" ]; then
        #add to ~/.zshrc if not added previously
        command="grep -qxF 'export VCPKG_ROOT="$HOME/vcpkg"' ~/.zshrc || echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc"
        #command="echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "VCPKG_ROOT added to PATH."

        command="grep -qxF 'export PATH=$PATH:"$HOME/vcpkg"' ~/.zshrc || echo 'export PATH=$PATH:"$HOME/vcpkg"' >> ~/.zshrc"
        eval "$command"
        #echo 'export PATH=$PATH:"$HOME/vcpkg"' >> ~/.zshrc
    fi
    my_echo_green "Add VAR to ENV : VCPKG_ROOT = $HOME/vcpkg "
}


#vcpkg bootstrap use curl,... so we should install them before bootstrap
install_linux_package(){
    if [ "$OS_NAME" == "Linux" ]; then
        my_echo_green "installing bootstrap needed packages..."
        command="sudo apt-get install curl zip unzip tar"
        eval "$command"
        check_last_command_exitcode_for_error "$command" "packages needed by bootstrap installed."
    fi
}

my_echo_green(){
    GREEN='\033[0;32m'
    NO_COLOR='\033[0m'
    echo -e "${GREEN}■ $1 ${NO_COLOR}"
}
my_echo_red(){
    RED='\033[31m'
    NO_COLOR='\033[0m'
    echo -e "${RED}■ $1 ${NO_COLOR}"
}
my_echo_yellow(){
    YELLOW='\033[0;33m'
    NO_COLOR='\033[0m'
    echo -e "${YELLOW}■ $1 ${NO_COLOR}"
}

install_vcpkg_package(){
    command="$HOME/vcpkg/vcpkg install $1 --triplet $2"
    eval "$command"
    check_last_command_exitcode_for_error "$command" "$1 -$2 installed successfully."
}
#Merging both macos versions into a universal binary
create_universal_bin_on_macos(){
    if [ "$OS_NAME" == "macOS" ]; then
        cd "$HOME/vcpkg/installed"   
        if [ "$1" == "" ]; then
            #need shutil
            command="python3 \"$script_path/lipo-dir-merge.py\" x64-osx arm64-osx uni-osx"
            eval "$command"
            check_last_command_exitcode_for_error "$command" "universal binary was built successfully."
        else
            command="python3 \"$script_path/lipo-dir-merge.py\" x64-osx-dynamic arm64-osx-dynamic uni-osx" 
            eval "$command"
            check_last_command_exitcode_for_error "$command" "universal binary was built successfully."
        fi   
        cd "$script_path"
    fi
}
check_last_command_exitcode_for_error(){
    if [ $? -ne 0 ]; then
        my_echo_red "Error: $1 failed."
        my_echo_red "Failed. VCPKG configuration terminate with error."
        exit 1
    else
         my_echo_green "$2"
    fi
}
check_last_command_exitcode_for_warning(){
    if [ $? -ne 0 ]; then
        my_echo_yellow "Warning: $1 failed."
    else
        my_echo_green "$2"
    fi
}

check_dep_file_exist(){
    if [ ! -f "$dependencies_file_path" ]; then
        my_echo_red " File [$dependencies_file_path] doesnt exist."
        my_echo_red "Failed. VCPKG configuration terminate with error."
        sleep 10
        exit 1
    fi
}

keep_console_open_in_windows(){
    if [ "$OS_NAME" == "Windows" ]; then
        read -p "Press any key to continue ..."
    fi  
}



#The main process start here
dependencies_file_path=$1
my_echo_green "configuration file path : $dependencies_file_path"
check_dep_file_exist
detect_os
adjust_git_safe_dir
init_vcpkg
add_to_env
install_dependencies
my_echo_green "Done. VCPKG successfully configured."
keep_console_open_in_windows
exit 0



