// FileName: bacon.c
// Author: Hamad Ayaz
// Purpose: Kevin Bacon Score simulator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ActorNode {
        char *name;
        struct MovieNode *movies;
        struct ActorNode *next;
        int visited;
        int level;
        struct ActorNode *parent;
} ActorNode;

typedef struct MovieNode {
        char *title;
        struct ActorNode *actors;
        struct MovieNode *next;
} MovieNode;

typedef struct QueueNode {
        ActorNode *actor;
        struct QueueNode *next;
} QueueNode;

MovieNode *globalMovieList = NULL;

// Utility function to create a new actor node
ActorNode* createActorNode(char *name) {
        ActorNode *newActor = (ActorNode*) malloc(sizeof(ActorNode));
        newActor->name = strdup(name);
        newActor->movies = NULL;
        newActor->next = NULL;
        newActor->visited = 0;
        newActor->level = -1;
        newActor->parent = NULL;
        return newActor;
}

// Adds an actor to the actors list if not already present
ActorNode* addActor(ActorNode **actors, char *name) {
        ActorNode *actor = findActor(*actors, name);
        if (actor == NULL) {
                actor = createActorNode(name);
                actor->next = *actors;
                *actors = actor;
        }
        return actor;
}

// Utility function to create a new movie node
MovieNode* createMovieNode(char *title) {
        MovieNode *newMovie = (MovieNode*) malloc(sizeof(MovieNode));
        newMovie->title = strdup(title);
        newMovie->actors = NULL;
        newMovie->next = NULL;
        return newMovie;
}

void linkActorToMovie(ActorNode *actor, MovieNode *movie) {
        // Link actor to movie's actor list
        ActorNode *actorInMovie = movie->actors;
        while (actorInMovie) {
                if (strcmp(actorInMovie->name, actor->name) == 0) {
                        return; // Actor already in movie's actor list
                }
                actorInMovie = actorInMovie->next;
        }
        ActorNode *newActorNode = (ActorNode*) malloc(sizeof(ActorNode));
        newActorNode->name = strdup(actor->name);
        newActorNode->movies = NULL;
        newActorNode->next = movie->actors;
        movie->actors = newActorNode;

        // Link movie to actor's movie list
        MovieNode *movieInActor = actor->movies;
        while (movieInActor) {
                if (strcmp(movieInActor->title, movie->title) == 0) {
                        return; // Movie already in actor's movie list
                }
                movieInActor = movieInActor->next;
        }
        MovieNode *newMovieNode = (MovieNode*) malloc(sizeof(MovieNode));
        newMovieNode->title = strdup(movie->title);
        newMovieNode->actors = NULL;
        newMovieNode->next = actor->movies;
        actor->movies = newMovieNode;
}

// Finds an actor in the list by name
ActorNode* findActor(ActorNode *actors, char *name) {
        while (actors != NULL) {
                if (strcmp(actors->name, name) == 0)
                        return actors;
                actors = actors->next;
        }
        return NULL;
}
MovieNode* findOrAddMovie(char *title) {
        // Check if movie already exists in global list
        for (MovieNode *movie = globalMovieList; movie != NULL; movie =
                        movie->next) {
                if (strcmp(movie->title, title) == 0) {
                        return movie;
                }
        }

        // Create new movie node and add to global list
        MovieNode *newMovie = (MovieNode*) malloc(sizeof(MovieNode));
        newMovie->title = strdup(title);
        newMovie->actors = NULL;
        newMovie->next = globalMovieList;
        globalMovieList = newMovie;
        return newMovie;
}

// Reads the movie file and builds the graph
void readMovieFile(char *filename, ActorNode **actors) {
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
                perror("Error opening file");
                exit(EXIT_FAILURE);
        }

        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        MovieNode *currentMovie = NULL;

        while ((read = getline(&line, &len, file)) != -1) {
                // Remove newline character
                if (line[read - 1] == '\n') {
                        line[read - 1] = '\0';
                }

                if (strncmp(line, "Movie: ", 7) == 0) {
                        char *title = line + 7;
                        currentMovie = findOrAddMovie(title); // find the movie in the global list or add if not found
                } else if (strlen(line) > 0 && currentMovie != NULL) {
                        ActorNode *actor = addActor(actors, line);
                        linkActorToMovie(actor, currentMovie);
                }
        }

        free(line); // Free the dynamically allocated memory
        fclose(file);
}

void printGraph(ActorNode *actors) {
        ActorNode *actor = actors;
        while (actor != NULL) {
                printf("Actor: %s\n", actor->name);
                MovieNode *movie = actor->movies;
                while (movie != NULL) {
                        printf("\tIn Movie: %s\n", movie->title);
                        movie = movie->next;
                }
                actor = actor->next;
        }
}

void enqueue(QueueNode **queue, ActorNode *actor) {
        QueueNode *newQueueNode = (QueueNode*) malloc(sizeof(QueueNode));
        newQueueNode->actor = actor;
        newQueueNode->next = NULL;

        if (*queue == NULL) {
                *queue = newQueueNode;
        } else {
                QueueNode *temp = *queue;
                while (temp->next != NULL) {
                        temp = temp->next;
                }
                temp->next = newQueueNode;
        }
}

ActorNode* dequeue(QueueNode **queue) {
        if (*queue == NULL) {
                return NULL;
        }

        QueueNode *temp = *queue;
        ActorNode *actor = temp->actor;
        *queue = (*queue)->next;
        free(temp);
        return actor;
}

int isQueueEmpty(QueueNode *queue) {
        return queue == NULL;
}

void resetGraph(ActorNode *actors) {
        ActorNode *temp = actors;
        while (temp != NULL) {
                temp->visited = 0;
                temp->level = -1;
                temp = temp->next;
        }
}

int computeBaconScore(ActorNode *actors, char *actorName) {
        // Check if the actor exists in the graph
        ActorNode *requestedActor = findActor(actors, actorName);
        if (requestedActor == NULL) {
                fprintf(stderr, "No actor named %s\n", actorName);
                return 0; // Actor not found
        }

        // Reset visited and level for all actors
        resetGraph(actors);

        // Find Kevin Bacon in the actor list
        ActorNode *kevinBacon = findActor(actors, "Kevin Bacon");
        if (kevinBacon == NULL) {
                printf("Score: No Bacon!\n");
                return 0; // Kevin Bacon not found
        }

        // Initialize queue for BFS
        QueueNode *queue = NULL;
        enqueue(&queue, kevinBacon);
        kevinBacon->visited = 1;
        kevinBacon->level = 0;

        while (!isQueueEmpty(queue)) {
                ActorNode *currentActor = dequeue(&queue);

                // Check all movies of the current actor
                for (MovieNode *actorMovie = currentActor->movies; actorMovie != NULL;
                                actorMovie = actorMovie->next) {
                        // Find the global movie node
                        MovieNode *globalMovie = findOrAddMovie(actorMovie->title);

                        // Check all actors in the global movie node
                        for (ActorNode *movieActor = globalMovie->actors;
                                        movieActor != NULL; movieActor = movieActor->next) {
                                ActorNode *globalActor = findActor(actors, movieActor->name);
                                if (!globalActor->visited) {
                                        globalActor->visited = 1;
                                        globalActor->level = currentActor->level + 1;
                                        globalActor->parent = currentActor; // for tracking the path
                                        enqueue(&queue, globalActor);
                                }
                        }
                }
        }

        // Find the requested actor and print their Bacon score
        if (requestedActor != NULL && requestedActor->visited) {
                printf("Score: %d\n", requestedActor->level);
        } else {
                printf("Score: No Bacon!\n");
        }

        return 1; // Successful
}

void cleanup(ActorNode *actors) {
        while (actors != NULL) {
                ActorNode *tempActor = actors;
                actors = actors->next;

                // Free movie list
                MovieNode *movies = tempActor->movies;
                while (movies != NULL) {
                        MovieNode *tempMovie = movies;
                        movies = movies->next;
                        free(tempMovie->title);
                        free(tempMovie);
                }

                // Free actor's name
                free(tempActor->name);

                // Free actor
                free(tempActor);
        }
}

MovieNode* findMovie(ActorNode *actors, char *title) {
        // Iterate through all actors and their movies to find the movie with the given title
        for (ActorNode *actor = actors; actor != NULL; actor = actor->next) {
                for (MovieNode *movie = actor->movies; movie != NULL;
                                movie = movie->next) {
                        if (strcmp(movie->title, title) == 0) {
                                return movie;
                        }
                }
        }
        return NULL; // Movie not found
}

void printMoviesAndActors(ActorNode *actors) {
        // Iterate through all actors
        for (ActorNode *actor = actors; actor != NULL; actor = actor->next) {
                printf("Actor: %s\n", actor->name);
                // Iterate through all movies of the actor
                for (MovieNode *movie = actor->movies; movie != NULL;
                                movie = movie->next) {
                        printf("\tMovie: %s\n", movie->title);
                        printf("\tActors in this movie:\n");
                        // Check if this movie is in the global list and print all actors from there
                        MovieNode *globalMovie = findMovie(actors, movie->title);
                        if (globalMovie) {
                                for (ActorNode *coActor = globalMovie->actors; coActor != NULL;
                                                coActor = coActor->next) {
                                        printf("\t\t%s\n", coActor->name);
                                }
                        }
                }
        }
}

// Main function
int main(int argc, char *argv[]) {
        int errorOccurred = 0; // Flag to track if an error occurred

        int showPath = 0; // Flag to determine if the path should be shown
        char *filename = NULL;

        // Parse command line arguments
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-l") == 0) {
                        showPath = 1;
                } else if (*argv[i] != '-' && filename == NULL) {
                        filename = argv[i];
                } else {
                        fprintf(stderr, "Usage: %s [-l] <filename>\n", argv[0]);
                        return EXIT_FAILURE;
                }
        }

        // Check if filename is provided
        if (filename == NULL) {
                fprintf(stderr, "Error: No input file specified.\n");
                return EXIT_FAILURE;
        }

        // Read the movie file and build the graph
        ActorNode *actors = NULL;
        readMovieFile(filename, &actors);

        if (showPath) {
        }

        // Read actor names from stdin and compute their Bacon scores
        char *actorName = NULL;
        size_t len = 0;

        while (getline(&actorName, &len, stdin) != -1) {
                if (actorName[strlen(actorName) - 1] == '\n') {
                        actorName[strlen(actorName) - 1] = '\0'; // Remove newline character
                }

                if (!computeBaconScore(actors, actorName)) {
                        errorOccurred = 1; // Mark that an error occurred
                }
        }

        free(actorName); // Free dynamically allocated memory
        cleanup(actors); // Cleanup resources
        return errorOccurred ? EXIT_FAILURE : EXIT_SUCCESS; // Exit with status 1 if an error occurred
}
