/*
 -------------------------------------
 File:    Huffman.c
 Project: Huffman Encode, Huffman Decode
 file description: - this file contains the required background functions
 to run the huffman encode and decode program.
 -------------------------------------
 Author: Lovette Oyewole
 Email: lovette.oyewole@icloud.com

 Version  2020-11-28

 Program Description:
 ===================
 Encode: Compresses text by coding the component symbols of a file based
 on character occurrence.
 Decode: Retrieves original text from a compressed text.

 -------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "huffman.h"

char path[128] = { }; // Global variable used to retrieve the path from the root to the leaf node

// Retrieve the frequencies of each symbol from the file
int *getFrequencies(FILE *f) {
	int curr;
	static int freq[128] = { 0 };
	int i;
	while (!feof(f)) {
		curr = fgetc(f);
		freq[curr]++;
	}
	return freq;
}

// Initialize a new huffman node
hNode* newhNode(char data, int freq) {
	hNode* node = (hNode*) malloc(sizeof(hNode));
	node->data = data;
	node->freq = freq;
	node->left = node->right = NULL;
	return node;
}

// Initialize a new priority queue node
pqNode* newpqNode(hNode *node) {
	pqNode* temp = (pqNode*) malloc(sizeof(pqNode));
	temp->data = node;
	temp->priority = node->freq;
	temp->next = NULL;

	return temp;
}

// Push a huffman node into the priority queue
pqNode *push(pqNode* head, hNode *node) {
	// If queue is empty
	if (head == NULL) {
		head = newpqNode(node);
	} else { // If queue is not empty
		pqNode* start = head;
		int priority = node->freq;

		// Create new Node
		pqNode* temp = newpqNode(node);

		// If the head of the queue has a lower
		// priority than new node, insert the new
		// node before head node and change head node
		// Priority is based off lowest frequency
		if (head->priority > priority) {

			// Insert New Node before head
			temp->next = head;
			head = temp;
		} else {
			// Traverse the queue and find the
			// position to insert the new node
			while (start->next != NULL && start->next->priority < priority) {
				start = start->next;
			}

			// Insert the node at the appriopriate posotion
			temp->next = start->next;
			start->next = temp;
		}
	}
	// Return the head of the priority queue
	return head;
}

// Create a parent node for two nodes with the parent node
// having a frequency equal to the sum of the frequencies of both child node
hNode *huffmanCombine(hNode *min1, hNode *min2) {
	// Create a new parent node
	hNode *root = newhNode('\0', min1->freq + min2->freq);
	// Assign two child nodes to the new parent node
	root->left = min1;
	root->right = min2;
	// Return the parent node
	return root;
}

// Get the path from the root to the leaf node
void getPath(hNode* root, char c, char *p) {
	// If the correct leaf node is found
	if (root->data == c) {
		strcpy(path, p);
		return;
	}

	char left[128] = { };
	strcpy(left, p);
	char right[128] = { };
	strcpy(right, p);

	// Check left sub-tree
	if (root->left != NULL
			&& (root->left->data == '\0' || root->left->data == c)) {
		getPath(root->left, c, strcat(left, "0"));
	}
	// Check right sub-tree
	if (root->right != NULL
			&& (root->right->data == '\0' || root->right->data == c)) {
		getPath(root->right, c, strcat(right, "1"));
	}

	return;
}

// Initialize the Huffman tree
hNode *initializeHuffmanTree(char *inputFile) {
	// Open the file containing the text to encode
	FILE *fptr;
	fptr = fopen(inputFile, "r");

	// Get array of frequencies of each symbol
	int *freq = getFrequencies(fptr);

	//Close file
	fclose(fptr);

	// Initialize a priority queue
	pqNode *head = NULL;

	// Create a huffman node for each symbol and their frequency
	// and insert the huffman node into the priority queue
	int i;
	for (i = 0; i < 128; i++) {
		if (freq[i] != 0) {
			head = push(head, newhNode((char) i, freq[i]));
		}
	}

	// Build the huffman tree
	hNode *min1, *min2;
	hNode *root;
	while (head->next != NULL) { // Loop until all nodes are combined to form 1 tree
		// Get min 1
		pqNode *pqn;
		min1 = head->data;
		pqn = head;
		head = head->next;
		pqn->next = NULL;
		free(pqn);

		// Get min 2
		pqNode *pqn2;
		min2 = head->data;
		pqn2 = head;
		head = head->next;
		pqn->next = NULL;
		free(pqn2);

		// Combine the 2 nodes with the lowest frequency
		root = huffmanCombine(min1, min2);
		// Insert the parent node retrieved from
		// combining the 2 nodes with the lowest
		// frequency back into the queue
		head = push(head, root);
	}

	// Get the root of the huffman tree from the
	// priority queue and free the priority queue
	root = head->data;
	free(head);

	return root;
}

// Frees the huffman tree
void free_memory(hNode *root) {
	// Check for left sub-tree
	if (root->left != NULL) {
		free_memory(root->left);
	}
	// Check for right sub-tree
	if (root->right != NULL) {
		free_memory(root->right);
	}
	// Frees the node
	root = NULL;
	free(root);
}

void encode(char *inputFile, char *outputFile) {

	// Open the file containing the text to encode
	FILE *fptrI;
	fptrI = fopen(inputFile, "r");

	// Open/create output file
	FILE *fptrO;
	fptrO = fopen(outputFile, "wb");

	// Build the Huffman tree
	hNode *root;
	root = initializeHuffmanTree(inputFile);

	// Encode the textfile
	char curr;
	if (fptrI != NULL) {
		while ((curr = fgetc(fptrI)) != EOF) {
			char p[100] = { };
			getPath(root, curr, p);
			//printf("%c - %s\n", curr, path);
			fprintf(fptrO, "%s", path);
		}
	} else {
		printf("Input file could not be opened");
	}

	// Close the files
	fclose(fptrI);
	fclose(fptrO);

	// Free the huffman tree
	free_memory(root);

	return;
}

void decode(char *inputFile, char *outputFile) {

	// Ask user for original file that was used to encode
	// in order to build the huffman tree

	printf("Enter the name of the original file that was used to encode : ");
	char original[30];
	scanf("%s", original);

	// Build the huffman tree
	hNode *root;
	root = initializeHuffmanTree(original);

	// Open the file containing the text to decode
	FILE *fptrI;
	fptrI = fopen(inputFile, "rb");

	// Open/create the file to write the decoded message
	FILE *fptrO;
	fptrO = fopen(outputFile, "w");

	// Check if file was opened
	if (fptrI != NULL) {
		char curr;
		hNode *temp;
		// Decode the encoded file
		while (!feof(fptrI)) {
			temp = root;
			while (temp->data == '\0' && (curr = fgetc(fptrI)) != EOF) {
				if (curr == '0') {
					temp = temp->left;
				} else if (curr == '1') {
					temp = temp->right;
				}
				//printf("%c", temp->data);
			}
			// Write the decoded symbol into the output file
			fprintf(fptrO, "%c", temp->data);
		}
	} else { // If file could not be opened
		printf("Input file could not be opened");
	}

	//Close the files
	fclose(fptrI);
	fclose(fptrO);

	//Free the huffman tree
	free_memory(root);

	return;
}
