/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hirwatan <hirwatan@student.42tokyo.jp>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/01 00:00:00 by user              #+#    #+#             */
/*   Updated: 2025/04/15 20:07:07 by hirwatan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "../libft/libft.h"
# include <ctype.h>
# include <dirent.h>
# include <errno.h>
# include <fcntl.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>

# define SINGLE_QUOTE_CHAR '\''
# define DOUBLE_QUOTE_CHAR '"'
# define SUCCESS 1
# define FAILED 0

extern volatile sig_atomic_t	g_signal;

typedef enum e_token_kind
{
	TK_WORD,
	TK_RESERVED,
	TK_EOF,
}								t_token_kind;

typedef struct s_token
{
	t_token_kind				kind;
	char						*word;
	struct s_token				*next;
}								t_token;

typedef enum e_redirect_kind
{
	RD_INPUT,
	RD_OUTPUT,
	RD_APPEND,
	RD_HEREDOC,
}								t_redirect_kind;

typedef struct s_redirect
{
	t_redirect_kind				kind;
	char						*filename;
	struct s_redirect			*next;
}								t_redirect;

typedef struct s_node
{
	char						**command;
	t_redirect					*redirects;
	struct s_node				*next;
}								t_node;

typedef struct s_env
{
	char						*key;
	char						*value;
	struct s_env				*next;
}								t_env;

typedef struct s_shell
{
	t_env						*env;
	t_node						*head;
	bool						syntax_error;
	int							status;
}								t_shell;

//---tokenize---
t_token							*tokenize(char *line, t_shell *shell);
int								is_reserved(char c);
t_token							*tokenize_reserved(char **line);
t_token							*tokenize_word(char **line, t_shell *shell);
char							*extract_word(char **line, t_shell *shell);
t_token							*new_token(t_token_kind kind, char *word);
void							add_token(t_token **tokens, t_token *new);
int								is_space(char c);

//---parse---
t_node							*parse(t_token *tokens, t_shell *shell);
t_node							*parse_command_node(t_token **token_ptr,
									t_shell *shell);
int								count_command_words(t_token *token);
char							**create_command_array(t_token *token,
									int count);
t_token							*handle_command(t_node *node, t_token *token);
t_token							*handle_redirect(t_node *node, t_token *token,
									t_shell *shell);
t_node							*new_node(void);
t_redirect						*new_redirect(t_redirect_kind kind,
									char *filename);
void							add_redirect_to_node(t_node *node,
									t_redirect *redirect);
t_token							*handle_redirects(t_node *node, t_token *token,
									t_shell *shell);
t_node							*create_redirect_node(t_token **token_ptr,
									t_shell *shell);

//---init---
t_env							*init_env(char **envp);
t_shell							*init_shell(t_env *env);
void							init_parse(t_node **current, t_node **head,
									t_node **next_node);

//---free---
void							free_all_nodes(t_node *head);
void							free_node(t_node *node);
void							free_tokens(t_token *tokens);
void							free_env(t_env *env);

//---signal---
void							signal_handler(int sig);
void							setup_signal(void);
void							signal_exit(int sig);
void							signal_handler_exit(int sig);
void							signal_pipe(int sig);

//---debug---
void							print_node(t_node *node);

//---minishell---
void							minishell_loop(t_shell *shell);

//---expand---
void							expand_variable(t_shell *shell);
bool							is_alpha_under(char c);
bool							is_alpha_num_under(char c);
bool							is_identifier(const char *s);
void							append_char(char **dst, char c);
bool							is_metacharacter(char c);
bool							is_variable(char *s);
char							*get_env_values(t_shell *shell,
									const char *key);
void							expand_variable_str(char **dst, char **rest,
									char *p, t_shell *shell);
void							expand_command(t_shell *shell, char **command);
void							expand_variable(t_shell *shell);
bool							is_special_parameter(char *s);
void							expand_special_parameter_str(char **dst,
									char **rest, char *p, t_shell *shell);
void							append_num(char **dst, unsigned int num);
void							append_single_quote(char **dst, char **rest,
									char *p);
void							append_double_quote(char **dst, char **rest,
									char *p, t_shell *shell);
char							*expand_string(t_shell *shell, char *word);
void							expand_redirects(t_shell *shell,
									t_redirect *redirect);

char							*get_env_value(t_env *env, char *key);
void							set_env_value(t_env *env, char *key,
									char *value);

int								is_builtin(char *cmd);
int								exec_builtin(char **cmd, t_env *env);
int								builtin_echo(char **args);
int								builtin_cd(char **args, t_env *env);
int								builtin_pwd(void);
int								builtin_export(char **args, t_env *env);
int								builtin_unset(char **args, t_env *env);
int								builtin_env(t_env *env);
int								builtin_exit(char **args);

char							**convert_env_to_array(t_env *env);

void							unset_env(t_env **env, char *key);

void							add_redirect(t_redirect **list,
									t_redirect *new);
t_redirect						*new_redirect(t_redirect_kind kind,
									char *filename);
t_redirect_kind					get_redirect_kind(t_token *token);
int								match(t_token *token, char *str);
t_env							*new_env_entry(char *key, char *value);
int								apply_input_redirect(t_redirect *redirect);
int								apply_output_redirect(t_redirect *redirect);
int								apply_append_redirect(t_redirect *redirect);
int								apply_heredoc(t_redirect *redirect);
void							execute_child(t_node *node, t_env *env);
char							*find_command_path(char *cmd, t_env *env);
void							execute_child(t_node *node, t_env *env);
int								apply_redirects(t_redirect *redirects);
int								execute_simple_command(t_node *node,
									t_env *env);

//---buildin---

//---redirect---
int								apply_redirections(t_redirect *current,
									int *fd_in, int *fd_out);
int								input_heredoc_redirect(t_redirect *current,
									int *fd_in);
int								open_input_redirect(t_redirect *redirect);
int								open_heredoc_redirect(t_redirect *redirect);

int								open_output_redirect(t_redirect *redirect);
int								open_append_redirect(t_redirect *redirect);
int								output_append_redirect(t_redirect *current,
									int *fd_out);

//---redirect---

char							*ft_strjoin(char const *s1, char const *s2);
int								execute_builtin_command(t_node *node,
									t_env *env);
int								buildin_unset(t_node *node, t_env **env);
void							unset_env(t_env **env, char *unset_key);
int								buildin_echo(t_node *node);
int								option_check(char *str);

int								buildin_cd(t_node *node, t_env *env);
char							*ft_getenv(char *str, t_env *env);
int								elements_check(char **str);
int								set_to_env_value(t_env *env, char *key,
									char *set_value);
char							*get_home_path(t_env *env);
int								buildin_cd(t_node *node, t_env *env);

int								buildin_exit(t_node *node);
int								is_numeric(char *str);
int								ft_atoull(const char *str,
									unsigned long long *result);
// static char						*extract_arg(char *line, int *index);
int								is_valid_number(char *arg,
									unsigned long long *num);

int								is_builtin(char *str);
int								execute_builtin_command(t_node *node,
									t_env *env);
int								buildin_export(t_node *node, t_env *env);
char							*ft_strjoin_safe(char *s1, char *s2);
int								is_valid_varname(char *name);
int								is_addition(char *arguments);
int								is_appending(char *arguments);
t_env							*find_env(t_env *env, char *key);
int								add_env(t_env *env, t_env *new_node);
t_env							*create_env_node(char *key, char *value);
void							export_valid_error(char *key, char *value,
									char *arguments);
void							env_addition(char *arguments, t_env *env);
void							export_join_env(t_env *existing, char *value,
									char *new_value, char *key);
void							env_appending(char *arguments, t_env *env);
int								no_assignment(char *arguments);
void							env_no_assignment(char *arguments, t_env *env);
int								print_env(t_env *env);
int								buildin_export(t_node *node, t_env *env);

int								buildin_env(t_env *env);
int								buildin_pwd(void);
int								execute_builtin_with_redirect(t_shell *shell,
									int *fd_in, int *fd_out);
//---buildin---

//---execute---
int								execute(t_shell *shell);

//---exec utils---
int								wexitstatus(int status);
int								wifexited(int status);
char							*join_three_strings(char const *s1,
									char const *s2, char const *s3);
char							*ft_strjoin(char const *s1, char const *s2);
char							**split_path_env(char *path);
char							**convert_env_to_array(t_env *env);
int								is_directry(char *str);
char							**create_path_array(t_env *env);
void							print_is_directory(char *str);
void							execute_direct_path(t_node *node, t_env *env);
void							search_command_in_path(t_node *node, t_env *env,
									char **path);
void							child_redirect(t_redirect *redirect,
									int *pipe_fd);
void							free_path_array(char **path);
void							free_env_array(char **envp);
int								ft_n_len(int n);
size_t							ft_strnlen(const char *s, size_t n);
char							*ft_strndup(const char *s, size_t n);

void							heredoc_eof_error(char *name, int i);
char							**create_path_array(t_env *env);
void							print_is_directory(char *str);
void							execute_direct_path(t_node *node, t_env *env);
void							search_command_in_path(t_node *node, t_env *env,
									char **path);
t_redirect						*connect_redirect_node(t_redirect *redirect,
									int stdin_fd, int stdout_fd);
int								execute_normal(t_node *node, t_env *env);
void							setup_redirections(t_redirect *redirect,
									int *local_fd_in, int *local_fd_out);
void							setup_standard_io(int local_fd_in,
									int local_fd_out, int fd_in);
pid_t							execute_pipeline_node(t_node *node, t_env *env,
									int fd_in, int *pipe_read_fd);
int								execute_pipeline(t_shell *shell);
void							execute_single_child(t_shell *shell, int fd_in,
									int fd_out);
void							execute_single(t_shell *shell, int fd_in,
									int fd_out);
int								execute(t_shell *shell);
char							*get_home_path(t_env *env);
char							*ft_getenv(char *str, t_env *env);
int								elements_check(char **str);
char							*get_array_path(char *str, t_env *env);
char							*ft_strtok(char *str, const char *delim);
int								ft_strcmp(char *s1, char *s2);
char							*ft_strstr(char *str, char *to_find);
void							setup_io_descriptors(int *local_fd_in,
									int *local_fd_out, int *pipe_out);
int								process_special_case(t_node *node);
void							execute_child_process(t_node *node, t_env *env,
									int fd_in, int *pipe_fd);
void							execute_single(t_shell *shell, int fd_in,
									int fd_out);

#endif