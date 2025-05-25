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
HTTP_DIR		= http_tcpServer/
FILE_DIR		= fileConfig/
BONUS_DIR       = bonus/
OBJ_DIR         = obj/

# -- Variables
COMPILED_FILES  = 0
LEN             = 0
FILE_FUNC		= CheckConfName ReadConfig ConfigUtils SetLocations
HTTP_FUNC	    = http_tcpServer_linux validateRequestMethod readRequest sendResponse setResponse startServer startListen shutDownServer acceptConnection runServer
SRC_FILES       = $(addprefix $(SRC_DIR)$(FILE_DIR), $(FILE_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR)$(HTTP_DIR), $(HTTP_FUNC:=.cpp)) \
					$(addprefix $(SRC_DIR), main.cpp) 

OBJS_SRC        = $(addprefix $(OBJ_DIR), $(SRC_FILES:%.cpp=%.o))
LIB             = libHttp_tcpServer_linux.a
CXX             = c++
CXXFLAGS        = -std=c++98
NAME            = webserv
TOTAL_FILES     = $(shell echo $$(($(words $(OBJS_SRC)))))
VALGRIND        = valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes
# MSG             = "[ $(COMPILED_FILES)/$(TOTAL_FILES) $$(($(COMPILED_FILES) * 100 / $(TOTAL_FILES)))%% ] $(ORANGE)Compiling [$1]... $(RESET)"
MSG 			= "$(CYAN) => ($(COMPILED_FILES)/$(TOTAL_FILES) $(PERCENT)%%) 🔧 Compiling [$1]...$(RESET)"

# -- Function to print the compilation message
define print_compile_msg
	$(eval COMPILED_FILES = $(shell echo $$(($(COMPILED_FILES) + 1))))
	$(eval PERCENT = $(shell echo $$(($(COMPILED_FILES) * 100 / $(TOTAL_FILES)))))
	$(eval LEN = $(shell echo -n $(MSG) | wc -c))
	@$(PRINT_CMD) "$$(printf '%*s\r' $(LEN) '')"
	@$(PRINT_CMD) $(MSG)
	@if [ $(COMPILED_FILES) -ne $(TOTAL_FILES) ]; then \
		$(PRINT_CMD) "\r" $(CUT) \
		$(PRINT_CMD) $(UP) $(CUT); \
	fi
endef

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
	$(call print_compile_msg,$<)
	@$(CXX) $(CXXFLAGS) -c $< -I./$(INCLUDE) -o $@

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
	@./$(NAME) ./conf_files/good/valid.conf

v:
	@make -s
	@$(VALGRIND) ./$(NAME) conf_files/good/valid.conf

fc: fclean

c: clean

# $(OBJ_DIR)main.o:	main.cpp $(HEADERS)
# 					$(call print_compile_msg,$<)
# 					@$(CXX) -c $< $(CXXFLAGS) -I./$(INCLUDE) -o $@