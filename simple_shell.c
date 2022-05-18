#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/**
* sh_launch - takes the list of args, then forks the process.
* @args: list of arguments
* Return: 1
*/
int sh_launch(char **args)
{
	pid_t pid;

	int status;

	pid = fork();

	if (pid == 0)
	{
		/* Child process*/
		if (execvp(args[0], args) == -1)
		{
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0)
	{
		/* Error forking*/
		perror("lsh");
	} else
	{
		/* Parent process*/
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return (1);
}

/**
* sh_split_line - have a buffer and expand it dynamically
* @line: pointer
* Return: tokens
*/
char **sh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;

	char **tokens = malloc(bufsize * sizeof(char *));

	char *token;

	if (!tokens)
	{
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;
		if (position >= bufsize)
		{
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if (!tokens)
			{
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, LSH_TOK_DELIM);
	}
	tokens[position] = NULL;
	return (tokens);
}

/**
* sh_read_line - Reading input from the user
*
* Return: Nothing
*/
char *sh_read_line(void)
{
	int bufsize = LSH_RL_BUFSIZE;

	int position = 0;

	char *buffer = malloc(sizeof(char) * bufsize);

	int c;

	if (!buffer)
	{
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	while (1)
	{
		/* Read a character */
		c = getchar();
		/* If we hit EOF, replace it with a null character and return.*/
		if (c == EOF || c == '\n')
		{
			buffer[position] = '\0';
			return (buffer);
		}
		buffer[position] = c;
		position++;
		/* If we have exceeded the buffer, reallocate.*/
		if (position >= bufsize)
		{
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer)
			{
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

int sh_exit(char **args);

int sh_cd(char **args);

char *builtin_str[] = { "exit", "cd" };

int (*builtin_func[]) (char **) = { &sh_exit, &sh_cd };

/**
* lsh_num_builtins - return size of str
* Return: Nothing
*/
int lsh_num_builtins(void)
{
	return (sizeof(builtin_str) / sizeof(char *));
}

/**
* sh_exit - used to exit prompt
* @args: array of arguments
* Return: O upon success
*/
int sh_exit(char **args)
{
	int i;

	i = 0;
	printf("%s\n", args[i]);
	return (0);
}

/**
* sh_cd - used to change dir
* @args: array of arguments
* Return: 1 upon success
*/
int sh_cd(char **args)
{
	if (args[1] == NULL)
	{
		chdir(getenv("HOME"));
		return (1);
	}
	if (chdir(args[1]) != 0)
	{
		perror("cd error");
	}
	return (1);
}


/**
* sh_execute - launches a builtin or a process
* @args: list of arguments
* Return: sh_launch(args)
*/
int sh_execute(char **args)
{
	int i;

	if (args[0] == NULL)
	{
		/* An empty command was entered.*/
		return (1);
	}
	for (i = 0; i < lsh_num_builtins(); i++)
	{
		if (strcmp(args[0], builtin_str[i]) == 0)
		{
			return ((*builtin_func[i])(args));
		}
	}
	return (sh_launch(args));
}
/**
* sh_loop - function that reads, parse and executes
*
* Return: Nothing
*/
void sh_loop(void)
{
	char *line;

	char **args;

	int status;

	do {
		printf("# ");
		line = sh_read_line();
		args = sh_split_line(line);
		status = sh_execute(args);
		free(line);
		free(args);
	} while (status);
}
/**
* main - Entry point of the simple shell program
* Return: Always 0 (Success)
*/
int main(void)
{
	sh_loop();
	return (0);
}
