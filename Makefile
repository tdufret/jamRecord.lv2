        user = $(shell whoami)
        ifeq ($(user),root)
        INSTALL_DIR = /usr/lib/lv2
        else 
        INSTALL_DIR = ~/.lv2
        endif

        NAME = jamRecord
        BUNDLE = $(NAME).lv2
        VER = 0.1

        CXXFLAGS = -I. -Wall -ffast-math  `pkg-config --cflags sndfile`
        LDFLAGS = -I. -I/usr/local/lib/lv2 -shared -Llibrary -lc -lm  -fPIC -DPIC `pkg-config --libs sndfile` -msse -mfpmath=sse 

        OBJECTS = $(NAME).c 

        # output style (bash colours)
        BLUE = "\033[1;34m"
        RED =  "\033[1;31m"
        NONE = "\033[0m"

.PHONY : all clean install uninstall 

all : $(NAME)
        @mkdir -p ./$(BUNDLE)
        @cp ./*.ttl ./$(BUNDLE)
        @mv ./*.so ./$(BUNDLE)
        @if [ -f ./$(BUNDLE)/$(NAME).so ]; then echo $(BLUE)"build finish, now run make install"; \
        else echo $(RED)"sorry, build failed"; fi
        @echo $(NONE)

clean :
        @rm -f $(NAME).so
        @rm -rf ./$(BUNDLE)
        @echo ". ." $(BLUE)", done"$(NONE)

install : all
        @mkdir -p $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)
        install ./$(BUNDLE)/* $(DESTDIR)$(INSTALL_DIR)/$(BUNDLE)
        @echo ". ." $(BLUE)", done"$(NONE)

uninstall :
        @rm -rf $(INSTALL_DIR)/$(BUNDLE)
        @echo ". ." $(BLUE)", done"$(NONE)

$(NAME) : 
        $(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) -o $(NAME).so
