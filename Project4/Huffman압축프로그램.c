#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct HuffNode {
    char data;
    int frequency;
    struct HuffNode* left;
    struct HuffNode* right;
};

struct PriorityQueue {
    int size;
    int capacity;
    struct HuffNode** elements;
};

struct HuffNode* createHuffNode(char data, int frequency, struct HuffNode* left, struct HuffNode* right) {
    struct HuffNode* node = (struct HuffNode*)malloc(sizeof(struct HuffNode));
    node->data = data;
    node->frequency = frequency;
    node->left = left;
    node->right = right;
    return node;
}

struct PriorityQueue* createPriorityQueue(int capacity) {
    struct PriorityQueue* queue = (struct PriorityQueue*)malloc(sizeof(struct PriorityQueue));
    queue->size = 0;
    queue->capacity = capacity;
    queue->elements = (struct HuffNode**)malloc(capacity * sizeof(struct HuffNode*));
    return queue;
}

struct HuffNode* dequeue(struct PriorityQueue* queue) {
    int i, j;
    struct HuffNode* temp = queue->elements[0];
    queue->elements[0] = queue->elements[queue->size - 1];
    queue->size--;

    i = 0;
    while (1) {
        j = i * 2 + 1;
        if (j >= queue->size)
            break;
        if (j + 1 < queue->size && queue->elements[j]->frequency > queue->elements[j + 1]->frequency)
            j++;
        if (queue->elements[i]->frequency <= queue->elements[j]->frequency)
            break;
        temp = queue->elements[i];
        queue->elements[i] = queue->elements[j];
        queue->elements[j] = temp;
        i = j;
    }
    return temp;
}

void enqueue(struct PriorityQueue* queue, struct HuffNode* node) {
    int i;
    queue->size++;
    i = queue->size - 1;
    while (i > 0 && node->frequency < queue->elements[(i - 1) / 2]->frequency) {
        queue->elements[i] = queue->elements[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    queue->elements[i] = node;
}

struct HuffNode* buildHuffmanTree(char* characters, int* frequencies, int size) {
    int i;
    struct HuffNode* left, * right, * top;
    struct PriorityQueue* queue = createPriorityQueue(size);

    for (i = 0; i < size; i++) {
        enqueue(queue, createHuffNode(characters[i], frequencies[i], NULL, NULL));
    }

    while (queue->size > 1) {
        left = dequeue(queue);
        right = dequeue(queue);
        top = createHuffNode('\0', left->frequency + right->frequency, left, right);
        enqueue(queue, top);
    }

    return dequeue(queue);
}

void calculateFrequencies(char* filename, char* characters, int* frequencies, int* size) {
    FILE* file = fopen(filename, "r");
    char c;

    if (file) {
        while ((c = fgetc(file)) != EOF) {
            int i;
            for (i = 0; i < *size; i++) {
                if (characters[i] == c) {
                    frequencies[i]++;
                    break;
                }
            }
            if (i == *size) {
                characters[*size] = c;
                frequencies[*size] = 1;
                (*size)++;
            }
        }
        fclose(file);
    }
}

void saveFrequencyStats(char* filename, char* characters, int* frequencies, int size) {
    FILE* file = fopen(filename, "w");
    int i;

    if (file) {
        for (i = 0; i < size; i++) {
            fprintf(file, "%c\t%d\n", characters[i], frequencies[i]);
        }
        fclose(file);
    }
}

void saveHuffmanCode(struct HuffNode* root, char* code, int index, FILE* file) {
    if (root->left) {
        code[index] = '0';
        saveHuffmanCode(root->left, code, index + 1, file);
    }

    if (root->right) {
        code[index] = '1';
        saveHuffmanCode(root->right, code, index + 1, file);
    }

    if (!root->left && !root->right) {
        int i;
        fprintf(file, "%c\t%s\n", root->data, code);
    }
}

void generateHuffmanCodes(char* filename, struct HuffNode* root) {
    FILE* file = fopen(filename, "w");
    char code[100];
    if (file) {
        saveHuffmanCode(root, code, 0, file);
        fclose(file);
    }
}

void compressFile(char* inputFilename, char* outputFilename, struct HuffNode* root) {
    FILE* inputFile = fopen(inputFilename, "r");
    FILE* outputFile = fopen(outputFilename, "wb");
    char c, buffer = 0;
    int bitCount = 0;

    if (inputFile && outputFile) {
        while ((c = fgetc(inputFile)) != EOF) {
            char code[100];
            int i;
            struct HuffNode* current = root;
            for (i = 0; i < 100; i++) {
                code[i] = '\0';
            }
            while (current) {
                if (current->left && strchr(current->left->data == c ? "1" : "0", '1')) {
                    strcat(code, "1");
                    current = current->left;
                }
                else if (current->right && strchr(current->right->data == c ? "1" : "0", '1')) {
                    strcat(code, "0");
                    current = current->right;
                }
                else {
                    break;
                }
            }
            for (i = 0; i < strlen(code); i++) {
                buffer <<= 1;
                buffer |= (code[i] == '1');
                bitCount++;
                if (bitCount == 8) {
                    fwrite(&buffer, sizeof(char), 1, outputFile);
                    buffer = 0;
                    bitCount = 0;
                }
            }
        }
        if (bitCount > 0) {
            buffer <<= (8 - bitCount);
            fwrite(&buffer, sizeof(char), 1, outputFile);
        }
        fclose(inputFile);
        fclose(outputFile);
    }
}

void decompressFile(char* inputFilename, char* outputFilename, struct HuffNode* root) {
    FILE* inputFile = fopen(inputFilename, "rb");
    FILE* outputFile = fopen(outputFilename, "w");
    struct HuffNode* current = root;
    unsigned char byte;
    int bitCount = 8;

    if (inputFile && outputFile) {
        while (fread(&byte, sizeof(unsigned char), 1, inputFile)) {
            int i;
            for (i = 7; i >= 0; i--) {
                if (bitCount == 0) {
                    current = root;
                    bitCount = 8;
                }
                if ((byte >> i) & 1) {
                    current = current->left;
                }
                else {
                    current = current->right;
                }

                if (!current->left && !current->right) {
                    fprintf(outputFile, "%c", current->data);
                    current = root;
                }
                bitCount--;
            }
        }
        fclose(inputFile);
        fclose(outputFile);
    }
}

int main() {
    char inputFilename[] = "input.txt";
    char statsFilename[] = "stats.txt";
    char compressedFilename[] = "output.huf";
    char decompressedFilename[] = "output.txt";
    char characters[256];
    int frequencies[256] = { 0 };
    int size = 0;
    struct HuffNode* root;

    calculateFrequencies(inputFilename, characters, frequencies, &size);
    saveFrequencyStats(statsFilename, characters, frequencies, size);

    root = buildHuffmanTree(characters, frequencies, size);
    generateHuffmanCodes(statsFilename, root);

    compressFile(inputFilename, compressedFilename, root);
    decompressFile(compressedFilename, decompressedFilename, root);

    return 0;
}
