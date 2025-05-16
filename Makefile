# -- Colors and formatting
RED				= \033[1;31m
YELLOW 			= \033[1;33m
ORANGE 			= \033[1;38;5;214m
GREEN 			= \033[1;32m
CYAN 			= \033[1;36m
RESET 			= \033[0m
UP				=	"\033[A"
CUT				=	"\033[K"

# Function to print the compilation message
define print_compile_msg
	$(eval COMPILED_FILES = $(shell echo $$(($(COMPILED_FILES) + 1))))
	@$(PRINT_CMD) "%105s\r"
	@$(PRINT_CMD) $(UP) $(CUT)
	@$(PRINT_CMD) $(MSG)
endef

define clean_func
	@if [ -d "$(OBJ_DIR)" ]; then \
		$(PRINT_CMD) "$(ORANGE)Removing '$(OBJ_DIR)' directory and main.o...$(RESET) "; \
		rm -rf $(OBJ_DIR); \
		rm -rf main.o; \
		$(PRINT_CMD) "$(GREEN)Removed successfully!$(RESET) ✅\n"; \
	fi
endef

define fclean_func
	@if [ -f "$(LIB)" ]; then \
		$(PRINT_CMD) "$(ORANGE)Removing '$(YELLOW)$(LIB)$(ORANGE)' library...$(RESET) "; \
		rm -f $(LIB); \
		$(PRINT_CMD) "$(GREEN)Removed successfully!$(RESET) ✅\n"; \
	fi
	@if [ -f "$(NAME)" ]; then \
		$(PRINT_CMD) "$(ORANGE)Removing '$(RED)$(NAME)$(ORANGE)' executable...$(RESET) "; \
		rm -f $(NAME); \
		$(PRINT_CMD) "$(GREEN)Removed successfully!$(RESET) ✅\n"; \
	fi
endef

# -- Files and directories
INCLUDE 		= inc/
HEADERS			= $(shell find $(INCLUDE) -name "*.hpp")
SRC_DIR 		= src/
BONUS_DIR 		= bonus/
OBJ_DIR 		= obj/

# -- Variables
COMPILED_FILES	= 0
PRINT_CMD		= printf
OS				= $(shell uname)
TOTAL_FILES		= $(shell echo $$(($(words $(OBJS_SRC)))))
VALGRIND		= valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes
MSG 			= "[ $(COMPILED_FILES)/$(TOTAL_FILES) $$(($(COMPILED_FILES) * 100 / $(TOTAL_FILES)))%% ] $(ORANGE)Compiling [$1]... $(RESET)"

# -- Compiler Settings
CXX				= c++
CXXFLAGS		= -std=c++98 #-Werror
NAME			= webServer
SRC_FILES 		= $(addprefix $(SRC_DIR), $(C_FUNCTIONS:=.cpp))
OBJS_SRC 		= $(addprefix $(OBJ_DIR), $(SRC_FILES:%.cpp=%.o))
LIB				= libwebserver.a
C_FUNCTIONS		= lets_go CheckConfName ReadConfig Configs ConfigUtils SetLocations

all:			$(NAME)

$(NAME): 		$(LIB) $(HEADERS)
				@$(CXX) $(CXXFLAGS) $(LIB) -o $(NAME)
				@echo "$(GREEN)Executable '$(RED)$(NAME)$(GREEN)' created successfully!$(RESET) ✅"

$(LIBFT_LIB):
				@make -s -C lib/libft/

$(LIB):			$(OBJS_SRC) $(OBJ_DIR)main.o
				@ar rcs $@ $(OBJS_SRC) $(OBJ_DIR)main.o
				@echo "$(CYAN)library '$(YELLOW)$(LIB)$(CYAN)' created successfully!$(RESET)"

$(OBJ_DIR)%.o:	%.cpp $(HEADERS)
				@mkdir -p $(dir $@)
				$(call print_compile_msg,$<)
				@$(CXX) $(CXXFLAGS) -c $< -I./$(INCLUDE) -o $@

$(OBJ_DIR)main.o:	main.cpp $(HEADERS)
					$(call print_compile_msg,$<)
					@$(CXX) -c $< $(CXXFLAGS) -I./$(INCLUDE) -o $@

clean:
				$(call clean_func)

fclean: 		clean
				@$(call fclean_func)

re: 			fclean all

bonus:			all

.PHONY: 		all clean fclean re bonus


r:
	@make -s
	@./$(NAME) ./map/ex1.cub

v:
	@make -s
	@$(VALGRIND) ./$(NAME) ./map/ex1.cub

fc: fclean

c: clean