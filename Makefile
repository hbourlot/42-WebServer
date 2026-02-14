# -- Colors and formatting
RED             = \033[1;31m
YELLOW          = \033[1;33m
ORANGE          = \033[1;38;5;214m
GREEN           = \033[1;32m
CYAN            = \033[1;36m
RESET           = \033[0m
WHITE_BOLD 		= \033[1;97m
UP              = "\033[A"
CUT				= "\033[K"
PRINT_CMD       = printf

# -- Files and directories
INCLUDE         = inc/
HEADERS         = $(shell find $(INCLUDE) -name "*.hpp")
SRC_DIR         = src/
FILE_DIR		= fileConfig/
AUTH_DIR		= auth/
UTILS_DIR		= utils/

CLIENT_DIR 		= client/
HTTP_RESPONSE_DIR = HttpResponse/
HTTP_DIR		= httpTcpServer/
REQUEST_DIR		= $(HTTP_DIR)request/
CGI_DIR			= $(HTTP_DIR)cgi/
UPL_DIR			= UploadManager/

BONUS_DIR       = bonus/
OBJ_DIR         = obj/

# -- Variables
COMPILED_FILES  = 0
LEN             = 0

CLIENT_FUNC		= client clientManager ClientEventProcessor ClientEventProcessor_cgi parseRequestData cookies
AUTH_FUNC		= loginHandler
CGI_FUNC		= Cgi buildEnvStrings
UTILS_FUNC		= utils getLocationFieldAsString debug
FILE_FUNC		= CheckConf ReadConfig ConfigUtils SetLocations SetFile
HTTP_FUNC	    = HttpTcpServerLinux Router autoIndex Request Response Logs
UPL_FUNC		= UploadManager parseMultipart

SRC_FILES       = $(addprefix $(SRC_DIR)$(FILE_DIR), $(FILE_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(CLIENT_DIR), $(CLIENT_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(CGI_DIR), $(CGI_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(HTTP_DIR), $(HTTP_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(UTILS_DIR), $(UTILS_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(AUTH_DIR), $(AUTH_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(UPL_DIR), $(UPL_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR), main.cpp) 

OBJS_SRC        = $(addprefix $(OBJ_DIR), $(SRC_FILES:%.cpp=%.o))
LIB             = libHttpTcpServerLinux.a
CXX             = c++
CXXFLAGS        = -std=c++98 -g #-Wall -Wextra
DEBUG_FLAGS		= -DDEBUG -g -O0
NAME            = webserv
TOTAL_FILES     = $(shell echo $$(($(words $(OBJS_SRC)))))
# VALGRIND        = valgrind --tool=massif
VALGRIND        = valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes
# MSG             = "[ $(COMPILED_FILES)/$(TOTAL_FILES) $$(($(COMPILED_FILES) * 100 / $(TOTAL_FILES)))%% ] $(ORANGE)Compiling [$1]... $(RESET)"
MSG 			= "$(CYAN) => ($(COMPILED_FILES)/$(TOTAL_FILES) $(PERCENT)%%) 🔧 Compiling [$1]...$(RESET)"

# -- Cleaning functions
define clean_func
	@if [ -d "$(OBJ_DIR)" ]; then \
		$(PRINT_CMD) "$(ORANGE) Removing '$(WHITE_BOLD)$(OBJ_DIR)$(ORANGE)'... $(RESET)"; \
		rm -rf $(OBJ_DIR); \
		rm -f main.o; \
		$(PRINT_CMD) "$(GREEN) Removed successfully!$(RESET)\n"; \
	fi
endef

define fclean_func
	@if [ -f "$(LIB)" ]; then \
		$(PRINT_CMD) "$(ORANGE) Removing library '$(WHITE_BOLD)$(LIB)$(ORANGE)'... $(RESET)"; \
		rm -f $(LIB); \
		$(PRINT_CMD) "$(GREEN) Removed successfully!$(RESET)\n"; \
	fi
	@if [ -f "$(NAME)" ]; then \
		$(PRINT_CMD) "$(ORANGE) Removing executable '$(WHITE_BOLD)$(NAME)$(ORANGE)'... $(RESET)"; \
		rm -f $(NAME); \
		$(PRINT_CMD) "$(GREEN) Removed successfully!$(RESET)\n"; \
	fi
endef

define check_completion
	@if [ $(COMPILED_FILES) -eq $(TOTAL_FILES) ]; then \
		$(PRINT_CMD) "$(GREEN)Compilation complete!$(RESET) ✅\n"; \
	fi
endef

# -- Targets
all: $(NAME)

$(NAME): $(LIB) $(HEADERS)
	@$(CXX) $(CXXFLAGS) $(LIB) -o $(NAME)
	@echo "$(GREEN) Executable '$(RED)$(NAME)$(GREEN)' created successfully!$(RESET) ✅"

$(LIB): $(OBJS_SRC)
	@ar rcs $@ $(OBJS_SRC)
	@echo "$(CYAN) library '$(YELLOW)$(LIB)$(CYAN)' created successfully!$(RESET)"

$(OBJ_DIR)%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(eval COMPILED_FILES = $(shell echo $$(($(COMPILED_FILES) + 1))))
	$(eval PERCENT = $(shell echo $$(($(COMPILED_FILES) * 100 / $(TOTAL_FILES)))))
	@echo "$(CYAN) => ($(COMPILED_FILES)/$(TOTAL_FILES) $(PERCENT)%) Compiling [$<]...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -I./$(INCLUDE) -o $@
	@printf ${UP}${CUT}

clean:
	$(call clean_func)

fclean: clean
	@$(call fclean_func)

re: fclean all

bonus: all

.PHONY: all clean fclean re bonus

# Shortcuts
r:
	@make -s
	@./$(NAME) ./conf_files/good/webpage.conf

s:
	@make -s
	@./$(NAME) ./conf_files/good/scriptpage.conf

v:
	@make -s
	@$(VALGRIND) ./$(NAME) ./conf_files/good/scriptpage.conf

debug: CXXFLAGS += $(DEBUG_FLAGS) -DDEBUG -g -O0
debug: re
	@echo "$(GREEN)Debug build complete!$(RESET)"

d: debug
	@make -s
	@./$(NAME) ./conf_files/good/scriptpage.conf

fc: fclean

c: clean

# Testing bad config formats
1:
	@echo "Bad extension test"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/badExtension.xaml

2:
	@echo "Checking Comment"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/CheckingComment.conf

3:
	@echo "Double bracket"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/doubleBracket.conf

4:
	@echo "Empty file .txt"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/empty.conf.txt

5:
	@echo "Empty .conf"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/empty.txt.conf

6:
	@echo "Missing information"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/MissingInformation.conf

7:
	@echo "No dote"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/no_dote

8:
	@echo "No type ."
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/no_type.

9:
	@echo "No closing brackets"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/noClosingBracket.conf

10:
	@echo "No error page"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/NoErrorPage.conf

11:
	@echo "No location"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/NoLocation.conf

12:
	@echo "No openning location"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/NoOpenningLocation.conf

13:
	@echo "No openning server"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/notOpenningServer.conf

14:
	@echo "No read permission"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/noReadPermission.conf

15:
	@echo "Double location"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/DoubleLocation.conf

16:
	@echo "Double server"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/DoubleServer.conf

17:
	@echo "Location without path"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/LocationWithNoPath.conf

18:
	@echo "Location with no root"
	@make -s
	@valgrind ./$(NAME) ./conf_files/bad/LocationWithNoRoot.conf

19:
	@echo "Location with specific file extension"
	@make -s
	@valgrind ./$(NAME) ./conf_files/good/file.conf

