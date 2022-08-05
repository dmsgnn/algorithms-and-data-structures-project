#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structure for array of pointers and strings
char ** text_pointers;
long int text_dimension = 0;

// structure for saving commands - these strings contains commands as it is received in input
// followed by eventual modified lines (for change or delete), detached by /n
char **commands_pointers;
int commands_dimension = 0;

// structure for delete commands
char *** delete_save;
long int * delete_save_dimension;

// structure for undo and redo commands
// this value tells if some undo/redo have been mixed - if mixed_ur is equal to one the necessary undo must be completed
int mixed_ur = 0;
int undo_counter = 0;
int gap;

struct structure{
    char *** mod_commands_pointers;
    char ** temp_text;
    long int temp_text_dimension;
};

void change (char * input);
void delete (char * input, long int delete_begin, long int delete_end);
void print (char * input);
void undo_count(char * input);
void save_delete ();
struct structure* undo (struct structure* data_structure);
struct structure* mod_save (char *input, struct structure* data_structure);
struct structure* pre_print (char * input, struct structure* data_structure);
struct structure* temp_undo (struct structure* data_structure);
struct structure* temp_delete (long int begin, long int end, struct structure* data_structure);


int main() {
    unsigned input_length;
    char input[1025];
    struct structure *data_structure;
    data_structure = (struct structure*) malloc(sizeof(struct structure));
    delete_save = (char ***) malloc(sizeof(char **));
    delete_save_dimension = (long int *) malloc(sizeof(long int));
    data_structure->mod_commands_pointers = (char ***) malloc(sizeof(char **));

    while(fgets(input, sizeof(input), stdin)){
        input_length = strlen(input);
        // quit
        if (input_length == 1 && input[0] == 'q')
            return 0;
        // change with mixed undo and redo
        if(input[input_length - 2] == 'c' && mixed_ur == 1){
            if(undo_counter != 0)
                undo(data_structure);
            mixed_ur=0;
        }
        // delete with mixed undo and redo
        if(input[input_length - 2] == 'd' && mixed_ur == 1){
            if(undo_counter != 0)
                undo(data_structure);
            mixed_ur=0;
        }
        // change or delete without command saved
        if(commands_dimension == 0 && (input[input_length - 2] == 'c' || input[input_length - 2] == 'd')){
            commands_pointers = (char **) malloc(sizeof(char *));
            commands_pointers[commands_dimension] = (char *) malloc((input_length + 1) * sizeof(char));
            memcpy(commands_pointers[commands_dimension], input, input_length + 1);
            commands_dimension += 1;
        }
        else{
            if(input[input_length - 2] == 'c' || input[input_length - 2] == 'd'){
                commands_pointers = realloc(commands_pointers, (commands_dimension + 1) * sizeof(char*));
                commands_pointers[commands_dimension] = (char *) malloc((input_length + 1) * sizeof(char));
                strcpy(commands_pointers[commands_dimension], input);
                commands_dimension += 1;
            }
        }
        if(input[input_length - 2] == 'c'){    // change
            if((commands_dimension) % 1024 == 0)
                save_delete();
            mod_save(input, data_structure);
            change(input);
        }
        else if (input[input_length - 2] == 'd'){  // delete
            save_delete();
            delete(input, 0, 0);
        }
        else if (input[input_length - 2] == 'p'){  // print
            if(mixed_ur == 1 && undo_counter != 0)
                pre_print(input, data_structure);
            else
                print(input);
        }
        else if (input[input_length - 2] == 'r'){  // redo
            if(mixed_ur == 1)
                undo_count(input);
        }
        else if (input[input_length - 2] == 'u'){  // undo
            if(mixed_ur == 0){
                mixed_ur=1;
            }
            undo_count(input);
        }
        else if (input_length == 2 && input[0] == 'q')   // quit
            return 0;
    }
    return 0;
}

void change (char * input){
    char change_input [1025];
    long int change_begin, change_end, i;
    int input_length;
    char *end, *command;
    change_begin=strtol(input, &end, 10);
    command = end+1;
    change_end=strtol(command, &end, 10);
    for(i= change_begin - 1; i <= change_end; i++){
        fgets(change_input, sizeof(change_input), stdin);
        input_length = strlen(change_input);
        if(input_length == 2 && change_input[0] == '.') {
            return;
        }
        if(text_dimension == 0 && i != 0)
            return;
        if(text_dimension == 0 && i == 0){
            text_pointers = (char **) malloc(sizeof(char *));
            text_pointers[text_dimension] = (char *) malloc((input_length + 1) * sizeof(char));
            memcpy(text_pointers[text_dimension], change_input, input_length + 1);
            text_dimension += 1;
        }
        else{
            if(i == text_dimension){
                text_pointers = realloc(text_pointers, (text_dimension + 1) * sizeof(char*));
                text_pointers[text_dimension] = (char*) malloc((input_length + 1) * sizeof(char));
                memcpy(text_pointers[text_dimension], change_input, input_length + 1);
                text_dimension += 1;
            }
            else{ // save deleted lines
                text_pointers[i] = (char*) malloc((input_length + 1) * sizeof(char));
                memcpy(text_pointers[i], change_input, input_length + 1);
            }
        }
    }
}

void delete (char * input, long int delete_begin, long int delete_end){
    long int  laps, i;
    unsigned new_length;

    if(input != NULL){
        char *end, *command;
        delete_begin = strtol(input, &end, 10);
        command = end + 1;
        delete_end = strtol(command, &end, 10);
    }
    if(delete_begin == 0 && delete_end == 0)
        return;
    if (text_dimension==0)
        return;
    if(delete_begin > text_dimension)
        return;
    if(delete_begin == 0 && delete_end != 0 )
        delete_begin = 1;
    if (delete_end > text_dimension)
        delete_end = text_dimension;
    laps = delete_end - delete_begin + 1;
    while(delete_begin - 1 + laps < text_dimension){
        new_length = strlen(text_pointers[delete_begin - 1 + laps]);
        text_pointers[delete_begin - 1] = (char*) malloc((new_length + 1) * sizeof(char));
        memcpy(text_pointers[delete_begin - 1], text_pointers[delete_begin - 1 + laps], new_length + 1);
        delete_begin++;
    }
    text_dimension -= laps;
    if(input != NULL) {
        text_pointers = realloc(text_pointers, (text_dimension) * sizeof(char *));
    }
    else{
        for(i=text_dimension; i < text_dimension + laps; i++)
            free(text_pointers[i]);
        if(text_dimension != 0)
            text_pointers = realloc(text_pointers, (text_dimension) * sizeof(char*));
        else
            text_pointers = (char**) malloc(sizeof(char*));
    }
}

struct structure* temp_delete (long int begin, long int end, struct structure* data_structure){
    long int laps;
    unsigned new_len;
    laps = end - begin + 1;
    if (data_structure->temp_text_dimension == 0)
        return data_structure;
    if(begin == 0 && end == 0)
        return data_structure;
    if(begin > data_structure->temp_text_dimension)
        return data_structure;
    if(begin == 0 && end != 0 )
        begin = 1;
    if (end > data_structure->temp_text_dimension)
        end = data_structure->temp_text_dimension;
    while(begin - 1 + laps < data_structure->temp_text_dimension){
        new_len = strlen(data_structure->temp_text[begin - 1 + laps]);
        data_structure->temp_text[begin - 1] = realloc(data_structure->temp_text[begin - 1], (new_len + 1) * sizeof(char));
        memcpy(data_structure->temp_text[begin - 1], data_structure->temp_text[begin - 1 + laps], new_len + 1);
        begin++;
    }
    data_structure->temp_text_dimension -= laps;
    if(data_structure->temp_text_dimension != 0)
        data_structure->temp_text = realloc(data_structure->temp_text, (data_structure->temp_text_dimension) * sizeof(char*));
    else
        data_structure->temp_text = (char**) malloc(sizeof(char*));
    return data_structure;
}



void print (char * input){
    long int print_begin, print_end, i;
    char *end, *command;
    print_begin=strtol(input, &end, 10);
    command = end+1;
    print_end=strtol(command, &end, 10);
    if(print_begin == 0 && print_end == 0) {
        fputs(".\n", stdout);
        return;
    }
    if(print_begin == 0 && print_end != 0) {
        fputs(".\n", stdout);
        print_begin=1;
    }
    for(i= print_begin - 1; i < print_end; i++){
        if(i >= text_dimension)
            fputs(".\n", stdout);
        else{
            fputs(text_pointers[i], stdout);
        }
    }
}

struct structure* mod_save (char *input, struct structure* data_structure){
    int save_begin, save_end, i, j, laps;
    char * end, *command;
    save_begin=strtol(input, &end, 10);
    command = end+1;
    save_end=strtol(command, &end, 10);
    laps = save_end - save_begin + 1;
    if(commands_dimension > 1)
        data_structure->mod_commands_pointers = realloc(data_structure->mod_commands_pointers, commands_dimension * sizeof(char**));
    data_structure->mod_commands_pointers[commands_dimension - 1] = (char**) malloc(laps * (sizeof(char*)));
    for(i= save_begin - 1, j=0; i < save_end || j < laps; i++, j++){
        if(text_dimension < i + 1){
            data_structure->mod_commands_pointers[commands_dimension - 1][j]=(char *)malloc(2 * sizeof(char));
            memcpy(data_structure->mod_commands_pointers[commands_dimension - 1][j], "*", 2);
        }
        if(text_dimension >= i + 1){
            data_structure->mod_commands_pointers[commands_dimension - 1][j] = text_pointers[i];
        }
    }
    return data_structure;
}

void undo_count(char * input){
    char * end;
    int laps, input_length;
    input_length = strlen(input);
    laps=strtol(input, &end, 10);
    if (input[input_length - 2] == 'u') {
        if (laps > commands_dimension)
            undo_counter += commands_dimension;
        else
            undo_counter += laps;
    }
    else if (input[input_length - 2] == 'r'){
        undo_counter -= laps;
    }
    if(undo_counter > commands_dimension)
        undo_counter = commands_dimension;
    if(undo_counter < 0)
        undo_counter=0;
}

void save_delete (){
    int i;
    if(commands_dimension > 1) {
        delete_save = realloc(delete_save, (commands_dimension) * sizeof(char **));
        delete_save_dimension = realloc (delete_save_dimension, (commands_dimension * sizeof(long int)));
    }
    delete_save[commands_dimension - 1] = (char**) malloc((text_dimension) * sizeof(char*));
    for(i=0; i < text_dimension; i++){
        delete_save[commands_dimension - 1][i]= (char*) malloc((strlen(text_pointers[i]) + 1) * sizeof(char));
        memcpy(delete_save[commands_dimension - 1][i], text_pointers[i], strlen(text_pointers[i]) + 1);
    }
    delete_save_dimension[commands_dimension - 1] = i;
}

struct structure* undo(struct structure* data_structure){
    long int i, len_com, j, undo_length, undo_begin, undo_end, undo_width, empty_lines_amount, s;
    char * end, * command;
    for(i=0; i < undo_counter; i++){
        len_com = strlen(commands_pointers[commands_dimension - 1 - i]);
        // delete undo
        if(commands_pointers[commands_dimension - 1 - i][len_com - 2] == 'd'){
            text_dimension = delete_save_dimension[commands_dimension - 1 - i];
            text_pointers = (char**)malloc(text_dimension * sizeof(char*));
            for(j=0; j < text_dimension; j++){
                undo_length = strlen(delete_save[commands_dimension - 1 - i][j]);
                text_pointers[j] = (char *)malloc ((undo_length + 1) * sizeof(char));
                memcpy(text_pointers[j], delete_save[commands_dimension - 1 - i][j], undo_length + 1);
            }
        }
        else if(commands_pointers[commands_dimension - 1 - i][len_com - 2] == 'c'){
            undo_begin=strtol(commands_pointers[commands_dimension - 1 - i], &end, 10);
            command = end+1;
            undo_end=strtol(command, &end, 10);
            undo_width = undo_end - undo_begin + 1;
            empty_lines_amount = 0;
            for(j=0; j < undo_width; j++){
                undo_length = strlen(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j]);
                if(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j][0] == '*' && undo_length == 1)
                    empty_lines_amount++;
            }
            if (empty_lines_amount == 0) {   // only non-empty lines
                for(j=0, s = undo_begin - 1; j < undo_width || s < undo_end; j++, s++){
                    undo_length = strlen(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j]);
                    text_pointers[s] = (char*) malloc((undo_length + 1) * sizeof(char));
                    memcpy(text_pointers[s], data_structure->mod_commands_pointers[commands_dimension - 1 - i][j], undo_length + 1);
                }
            }
            else if (empty_lines_amount == undo_width) {  // only empty line (asterisk are used to represent empty lines)
                delete(NULL, undo_begin, undo_end);
            }
            else if(empty_lines_amount != undo_width){  // mixed lines
                delete(NULL, undo_end - empty_lines_amount + 1, undo_end);
                for(j=0, s = undo_begin - 1; s < undo_end - empty_lines_amount; j++, s++){
                    undo_length = strlen(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j]);
                    text_pointers[s] = (char*) malloc((undo_length + 1) * sizeof(char));
                    memcpy(text_pointers[s], data_structure->mod_commands_pointers[commands_dimension - 1 - i][j], undo_length + 1);
                }
            }
        }
    }
    if(undo_counter != 0){
        for(i=commands_dimension; i > commands_dimension - undo_counter; i--){
            long int begin_line = strtol(commands_pointers[i - 1], &end, 10);
            command = end+1;
            long int end_line = strtol(command, &end, 10);
            long int laps = end_line - begin_line + 1;
            if(commands_pointers[i - 1][strlen(commands_pointers[i - 1]) - 2] == 'c')
                for(j=0; j<laps; j++)
                    free(data_structure->mod_commands_pointers[i - 1][j]);
            if(commands_pointers[i - 1][strlen(commands_pointers[i - 1]) - 2] == 'd')
                for(j=0; j < delete_save_dimension[i - 1]; j++)
                    free(delete_save[i - 1][j]);
            free(commands_pointers[i - 1]);
        }
        commands_dimension -= undo_counter;
        if(commands_dimension != 0){
            delete_save = realloc(delete_save, (commands_dimension) * sizeof(char **));
            commands_pointers = realloc(commands_pointers, (commands_dimension) * sizeof(char *));
            data_structure->mod_commands_pointers = realloc(data_structure->mod_commands_pointers, (commands_dimension) * sizeof(char **));
            delete_save_dimension = realloc(delete_save_dimension, commands_dimension * sizeof(long int));
            undo_counter = 0;
        }
        else{
            delete_save = (char***) malloc(sizeof(char **));
            commands_pointers = (char** )malloc(sizeof(char *));
            data_structure->mod_commands_pointers = (char***) malloc(sizeof(char **));
            delete_save_dimension = (long int*) malloc(sizeof(long int));
            undo_counter = 0;
        }
    }
    return data_structure;
}

struct structure* temp_undo (struct structure *data_structure){
    int i, len_com, j, temp_undo_length, temp_undo_begin, temp_undo_end, laps, empty_lines_amount, s;
    char * end, *command;
    for(i=0+gap; i < undo_counter; i++){
        len_com = strlen(commands_pointers[commands_dimension - 1 - i]);
        if(commands_pointers[commands_dimension - 1 - i][len_com - 2] == 'd'){
            data_structure->temp_text_dimension = delete_save_dimension[commands_dimension - 1 - i];
            data_structure->temp_text = realloc(data_structure->temp_text, data_structure->temp_text_dimension * sizeof(char*));
            for(j=0; j < data_structure->temp_text_dimension; j++){
                data_structure->temp_text[j] = delete_save[commands_dimension - 1 - i][j];
            }
        }
        else if(commands_pointers[commands_dimension - 1 - i][len_com - 2] == 'c'){
            temp_undo_begin=strtol(commands_pointers[commands_dimension - 1 - i], &end, 10);
            command = end+1;
            temp_undo_end=strtol(command, &end, 10);
            laps = temp_undo_end - temp_undo_begin + 1;
            empty_lines_amount = 0;
            for(j=laps-1; j>=0; j--){
                if(data_structure->mod_commands_pointers[commands_dimension - 1 - i][laps - 1][0] == '*' && data_structure->mod_commands_pointers[commands_dimension - 1 - i][0][0] == '*'){
                    empty_lines_amount = laps;
                    break;
                }
                temp_undo_length = strlen(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j]);
                if(temp_undo_length == 1 && data_structure->mod_commands_pointers[commands_dimension - 1 - i][j][0] == '*')
                    empty_lines_amount++;
                if(data_structure->mod_commands_pointers[commands_dimension - 1 - i][j][0] != '*')
                    break;
            }
            if (empty_lines_amount == 0) {
                for(j=0, s = temp_undo_begin - 1; j < laps || s < temp_undo_end; j++, s++){
                    data_structure->temp_text[s] = data_structure->mod_commands_pointers[commands_dimension - 1 - i][j];
                }
            }
            else if (empty_lines_amount == laps) {
                data_structure = temp_delete(temp_undo_begin, temp_undo_end, data_structure);
            }
            else if(empty_lines_amount != laps) {
                data_structure = temp_delete(temp_undo_end - empty_lines_amount + 1, temp_undo_end, data_structure);
                for(j=0, s = temp_undo_begin - 1; s < temp_undo_end - empty_lines_amount; j++, s++){
                    data_structure->temp_text[s] = data_structure->mod_commands_pointers[commands_dimension - 1 - i][j];
                }
            }
        }
    }
    return data_structure;
}

struct structure* pre_print (char * input, struct structure* data_structure){
    int pre_print_begin, pre_print_end, j, i;
    char * end, *command;
    pre_print_begin=strtol(input, &end, 10);
    command = end+1;
    pre_print_end=strtol(command, &end, 10);
    if(undo_counter == commands_dimension){
        for(i=0; i < pre_print_end; i++)
            fputs(".\n", stdout);
        return data_structure;
    }
    gap = 0;
    if(commands_pointers[commands_dimension - undo_counter][(strlen(commands_pointers[commands_dimension - undo_counter])) - 2] == 'd'){
        for(i=0; i < pre_print_end; i++)
            if(i < delete_save_dimension[commands_dimension - undo_counter])
                fputs(delete_save[commands_dimension - undo_counter][i], stdout);
            else
                fputs(".\n", stdout);
        return data_structure;
    }
    if(((commands_dimension - undo_counter % 256) == 0) && (commands_pointers[commands_dimension - undo_counter][(strlen(commands_pointers[commands_dimension - undo_counter])) - 2] == 'c')){
        for(i=0; i < pre_print_end; i++)
            if(i < delete_save_dimension[commands_dimension - undo_counter])
                fputs(delete_save[commands_dimension - undo_counter][i], stdout);
            else
                fputs(".\n", stdout);
        return data_structure;
    }
    for(i=undo_counter; i > 0; i--){
        if(commands_pointers[commands_dimension - i][(strlen(commands_pointers[commands_dimension - i])) - 2] == 'd') {
            gap = i;
            if (delete_save_dimension[commands_dimension - i] != 0) {
                data_structure->temp_text = (char **) malloc((delete_save_dimension[commands_dimension - i]) * sizeof(char *));
                for (j = 0; j < delete_save_dimension[commands_dimension - i]; j++) {
                    data_structure->temp_text[j] = delete_save[commands_dimension - i][j];
                }
            }
            data_structure->temp_text_dimension = delete_save_dimension[commands_dimension - i];
            data_structure = temp_undo(data_structure);
            if(pre_print_begin == 0 && pre_print_end == 0) {
                fputs(".\n", stdout);
                return data_structure;
            }
            if(pre_print_begin == 0) {
                fputs(".\n", stdout);
                pre_print_begin=1;
            }
            for(i= pre_print_begin - 1; i < pre_print_end; i++){
                if(i >= data_structure->temp_text_dimension)
                    fputs(".\n", stdout);
                else{
                    fputs(data_structure->temp_text[i], stdout);
                }
            }
            return data_structure;
        }
        if(((commands_dimension - i + 1) % 1024 == 0) && commands_pointers[commands_dimension - i][(strlen(commands_pointers[commands_dimension - i])) - 2] == 'c') {
            gap = i;
            if (delete_save_dimension[commands_dimension - i] != 0) {
                data_structure->temp_text = (char **) malloc((delete_save_dimension[commands_dimension - i]) * sizeof(char *));
                for (j = 0; j < delete_save_dimension[commands_dimension - i]; j++) {
                    data_structure->temp_text[j] = delete_save[commands_dimension - i][j];
                }
            }
            data_structure->temp_text_dimension = delete_save_dimension[commands_dimension - i];
            data_structure = temp_undo(data_structure);
            if(pre_print_begin == 0 && pre_print_end == 0) {
                fputs(".\n", stdout);
                return data_structure;
            }
            if(pre_print_begin == 0) {
                fputs(".\n", stdout);
                pre_print_begin=1;
            }
            for(i= pre_print_begin - 1; i < pre_print_end; i++){
                if(i >= data_structure->temp_text_dimension)
                    fputs(".\n", stdout);
                else{
                    fputs(data_structure->temp_text[i], stdout);
                }
            }
            return data_structure;
        }
    }
    gap = 0;
    data_structure->temp_text_dimension = text_dimension;
    if(data_structure->temp_text_dimension != 0)
        data_structure->temp_text = (char**) malloc ((data_structure->temp_text_dimension) * sizeof(char*));
    else
        data_structure->temp_text = (char**) malloc (sizeof(char*));
    if(data_structure->temp_text_dimension != 0) {
        for (i = 0; i < data_structure->temp_text_dimension; i++) {
            data_structure->temp_text[i] = text_pointers[i];
        }
    }
    data_structure= temp_undo(data_structure);
    if(pre_print_begin == 0 && pre_print_end == 0) {
        fputs(".\n", stdout);
        return data_structure;
    }
    if(pre_print_begin == 0) {
        fputs(".\n", stdout);
        pre_print_begin=1;
    }
    for(i= pre_print_begin - 1; i < pre_print_end; i++){
        if(i >= data_structure->temp_text_dimension)
            fputs(".\n", stdout);
        else{
            fputs(data_structure->temp_text[i], stdout);
        }
    }
    return data_structure;
}