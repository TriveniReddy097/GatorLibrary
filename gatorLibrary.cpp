#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;
// #define outputFileName "output_file.txt"
#define INT_MAX   __INT_MAX__

struct ReservationNode {
    int patronId;
    int priorityNumber;
    long long timeOfReservation;

    ReservationNode(int id, int priority, long long timeStamp) : patronId(id), priorityNumber(priority), 
    timeOfReservation(timeStamp) {};
};

struct Book {
    int bookId;
    int borrowedBy;
    string bookName;
    string authorName;
    int availabilityStatus;

    vector<ReservationNode> reservationHeap;
};

struct RBTreeNode {
    Book book;
    int color; // 1  for red, 0 for black
    int index;
    RBTreeNode * left;
    RBTreeNode * right;
    RBTreeNode * parent;
    RBTreeNode * linkNodePtr;
};

class gatorLibrary {
private:
    RBTreeNode * root;
    int flipCount;
    RBTreeNode * NULLNODE;
    ofstream outputFile;
    vector<ReservationNode> reservationHeap;
    string outputFileName; // Member variable to hold the dynamically generated output file name

    // Function to set the output file name based on input file name
    void setOutputFileName(const string& inputFileName) {
        size_t dotPos = inputFileName.find_last_of('.');
        string fileNameWithoutExt = inputFileName.substr(0, dotPos);
        outputFileName = fileNameWithoutExt + "_output_file.txt";
    }

    // Open the output file based on the stored output file name
    void openOutputFile() {
        outputFile.open(outputFileName);
    }

    // Close the output file
    void closeOutputFile() {
        outputFile.close();
    }
    // Fix any violations of the Red-Black Tree properties that may have occurred after inserting nodeK
    void insertRBTreeNodeFix(RBTreeNode * nodeK) {
        RBTreeNode * nodeU;

        // Continue fixing violations until there are no more violations or the root is reached
        while (nodeK -> parent && nodeK -> parent -> color == 1) {
            if (nodeK -> parent == nodeK -> parent -> parent -> right) {
                // Set nodeU to the left child of the grandparent of nodeK
                nodeU = nodeK -> parent -> parent -> left;
                if (nodeU -> color == 1) {
                    // Set the colors of nodeU, its parent, and its grandparent
                    nodeU -> color = 0;
                    nodeK -> parent -> color = 0;
                    nodeK -> parent -> parent -> color = 1;

                    // Continue fixing violations with the grandparent of nodeK
                    nodeK = nodeK -> parent -> parent;
                    flipCount++;
                }
                else {
                    if (nodeK == nodeK -> parent -> left) {
                        // Continue fixing violations with the parent of nodeK
                        nodeK = nodeK -> parent;
                        // Rotate right around the parent of nodeK three
                        rbTreeRightRotate(nodeK);
                    }
                    // Set the colors of nodeK's parent and grandparent
                    nodeK -> parent -> color = 0;
                    nodeK -> parent -> parent -> color = 1;
                    flipCount++;
                    // Rotate left around the grandparent of nodeK
                    rbTreeLeftRotate(nodeK -> parent -> parent);
                }
            }
            else {
                // Set nodeU to the right child of the grandparent of nodeK
                nodeU = nodeK -> parent -> parent -> right;
                if (nodeU -> color == 1) {
                    // Set the colors of nodeU, its parent, and its grandparent
                    nodeU -> color = 0;
                    nodeK -> parent -> color = 0;
                    nodeK -> parent -> parent -> color = 1;
                    flipCount++;
                    // Continue fixing violations with the grandparent of nodeK
                    nodeK = nodeK -> parent -> parent;
                }
                else {
                    if (nodeK == nodeK -> parent -> right) {
                        // Continue fixing violations with the parent of nodeK
                        nodeK = nodeK -> parent;
                        // Rotate left around the parent of nodeK
                        rbTreeLeftRotate(nodeK);
                    }
                    // Set the colors of nodeK's parent and grandparent
                    nodeK -> parent -> color = 0;
                    nodeK -> parent -> parent -> color = 1;
                    flipCount++;
                    // Rotate right around the grandparent of nodeK
                    rbTreeRightRotate(nodeK -> parent -> parent);
                }
            }
            if (nodeK == root) {
                break;
            }
        }
        root -> color = 0;
        flipCount++;
    }

    RBTreeNode* searchRBTreeHelper(RBTreeNode* rbNode, int bookId) {
        // If the node pointer is NULL or the bookId of the node matches the search bookId, return the node
        if (rbNode == NULLNODE || bookId == rbNode->book.bookId)
            return rbNode;

        // If the search bookId is greater than or equal to the bookId of the current node,
        // search in the right subtree of the current node
        if (bookId >= rbNode->book.bookId)
            return searchRBTreeHelper(rbNode->right, bookId);

        // If the search bookId is less than the bookId of the current node,
        // search in the left subtree of the current node
        return searchRBTreeHelper(rbNode->left, bookId);
    }

    int getCurrentTimestamp() {
        // Get the current time point
        auto now = chrono::system_clock::now();

        // Convert the time point to milliseconds since the epoch
        auto duration = now.time_since_epoch();
        auto milliseconds = chrono::duration_cast<chrono::milliseconds>(duration);

        // Return the current timestamp in milliseconds
        return static_cast<int>(milliseconds.count());
    }

    // HeapifyUp function for a min-heap
    void heapifyUp(vector<ReservationNode>& heap, int index) {
        while (index > 0) {
            int parentIndex = (index - 1) / 2;

            // Compare the priority of the current node with its parent
            if (heap[index].priorityNumber < heap[parentIndex].priorityNumber ||
                (heap[index].priorityNumber == heap[parentIndex].priorityNumber &&
                heap[index].timeOfReservation < heap[parentIndex].timeOfReservation)) {
                // Swap the current node with its parent
                swap(heap[index], heap[parentIndex]);
                index = parentIndex; // Move up to the parent index
            } else {
                break; // Stop if the heap property is satisfied
            }
        }
    }

    void heapifyDown(vector<ReservationNode>& heap, int index) {
        int size = heap.size();
        int smallest = index;

        while (true) {
            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;

            // Compare with left child
            if (leftChild < size &&
                (heap[leftChild].priorityNumber < heap[smallest].priorityNumber ||
                (heap[leftChild].priorityNumber == heap[smallest].priorityNumber &&
                heap[leftChild].timeOfReservation < heap[smallest].timeOfReservation))) {
                smallest = leftChild;
            }

            // Compare with right child
            if (rightChild < size &&
                (heap[rightChild].priorityNumber < heap[smallest].priorityNumber ||
                (heap[rightChild].priorityNumber == heap[smallest].priorityNumber &&
                heap[rightChild].timeOfReservation < heap[smallest].timeOfReservation))) {
                smallest = rightChild;
            }

            // If the smallest is not the current index, swap and continue
            if (smallest != index) {
                swap(heap[index], heap[smallest]);
                index = smallest;
            } else {
                break;  // Stop if the heap property is satisfied
            }
        }
    }

    void inOrder(RBTreeNode* node, vector<Book>& listOfBooks) {
        if (node == nullptr) {
            return;
        }
        inOrder(node->left, listOfBooks);
        listOfBooks.push_back(node->book);
        inOrder(node->right, listOfBooks);
    }

    RBTreeNode* minimumRBTreeNode(RBTreeNode* rbTreeNode) {
        // Follow the left child pointers until we reach a node with no left child.
        while (rbTreeNode->left != NULLNODE) {
            rbTreeNode = rbTreeNode->left;
        }
        // Return the node with the minimum key.
        return rbTreeNode;
    }

    // Perform a left rotation on the red-black tree at nodeX.
    void rbTreeLeftRotate(RBTreeNode * nodeX) {
        // Store nodeY with the right child of nodeX.
        RBTreeNode * nodeY = nodeX -> right;
        // Set the right child of nodeX to be the left child of nodeY.
        nodeX -> right = nodeY -> left;
        // Update the parent of the left child of nodeY to be nodeX.
        if (nodeY -> left != NULLNODE) {
            nodeY -> left -> parent = nodeX;
        }
        // Update the parent of nodeY to be the parent of nodeX.
        nodeY -> parent = nodeX -> parent;
        // If nodeX is the root of the tree, set nodeY as the new root.
        if (nodeX -> parent == nullptr) {
            this -> root = nodeY;
        }
        // If nodeX is the left child of its parent, set nodeY as the left child of the parent.
        else if (nodeX == nodeX -> parent -> left) {
            nodeX -> parent -> left = nodeY;
        }
        // Otherwise, set nodeY as the right child of the parent.
        else {
            nodeX -> parent -> right = nodeY;
        }
        // Set nodeX as the left child of nodeY and nodeY as the parent of nodeX.
        nodeY -> left = nodeX;
        nodeX -> parent = nodeY;
    }

    // Perform a right rotation on the given node in the red-black tree.
    // Parameters: nodeX: The node to rotate.
    void rbTreeRightRotate(RBTreeNode * nodeX) {
        // Set nodeY to the left child of nodeX.
        RBTreeNode * nodeY = nodeX -> left;
        // Set the left child of nodeX to be the right child of nodeY.
        nodeX -> left = nodeY -> right;
        if (nodeY -> right != NULLNODE) {
            // Set the parent of the right child of nodeY to be nodeX, if it exists.
            nodeY -> right -> parent = nodeX;
        }
        // Set the parent of nodeY to be the parent of nodeX.
        nodeY -> parent = nodeX -> parent;
        if (nodeX -> parent == nullptr) {
            // If nodeX is the root of the tree, set the root to be nodeY.
            this -> root = nodeY;
        } else if (nodeX == nodeX -> parent -> right) {
            // If nodeX is the right child of its parent, set the right child of the parent to be nodeY.
            nodeX -> parent -> right = nodeY;
        } else {
            // Otherwise, set the left child of the parent to be nodeY.
            nodeX -> parent -> left = nodeY;
        }
        // Set the right child of nodeY to be nodeX, and set the parent of nodeX to be nodeY.
        nodeY -> right = nodeX;
        nodeX -> parent = nodeY;
    }

    void deleteRBTreeNodeFix(RBTreeNode* nodeX) {
        while (nodeX != root && nodeX->color == 0 && nodeX->parent != nullptr) {
            if (nodeX == nodeX->parent->left) {
                RBTreeNode* sibling = nodeX->parent->right;

                if (sibling != nullptr) {
                    if (sibling->color == 1) {
                        sibling->color = 0;
                        flipCount++;
                        nodeX->parent->color = 1;
                        rbTreeLeftRotate(nodeX->parent);
                        sibling = nodeX->parent->right;
                    }

                    if (sibling != nullptr && sibling->left != nullptr && sibling->right != nullptr &&
                        sibling->left->color == 0 && sibling->right->color == 0) {
                        sibling->color = 1;
                        flipCount++;
                        nodeX = nodeX->parent;
                    } else {
                        if (sibling != nullptr && sibling->right != nullptr && sibling->right->color == 0) {
                            if (sibling->left != nullptr) {
                                sibling->left->color = 0;
                                flipCount++;
                            }
                            if (sibling != nullptr) {
                                sibling->color = 1;
                                flipCount++;
                            }
                            rbTreeRightRotate(sibling);
                            sibling = nodeX->parent->right;
                        }
                        if (sibling != nullptr) {
                            sibling->color = nodeX->parent->color;
                        }
                        if (nodeX->parent != nullptr) {
                            nodeX->parent->color = 0;
                            flipCount++;
                        }
                        if (sibling != nullptr && sibling->right != nullptr) {
                            sibling->right->color = 0;
                            flipCount++;
                        }
                        if (nodeX->parent != nullptr) {
                            rbTreeLeftRotate(nodeX->parent);
                        }
                        nodeX = root;
                    }
                }
            } else {
                RBTreeNode* sibling = nodeX->parent->left;

                if (sibling != nullptr) {
                    if (sibling->color == 1) {
                        sibling->color = 0;
                        flipCount++;
                        nodeX->parent->color = 1;
                        rbTreeRightRotate(nodeX->parent);
                        sibling = nodeX->parent->left;
                    }

                    if (sibling != nullptr && sibling->right != nullptr && sibling->left != nullptr &&
                        sibling->right->color == 0 && sibling->left->color == 0) {
                        sibling->color = 1;
                        flipCount++;
                        nodeX = nodeX->parent;
                    } else {
                        if (sibling != nullptr && sibling->left != nullptr && sibling->left->color == 0) {
                            if (sibling->right != nullptr) {
                                sibling->right->color = 0;
                                flipCount++;
                            }
                            if (sibling != nullptr) {
                                sibling->color = 1;
                                flipCount++;
                            }
                            rbTreeLeftRotate(sibling);
                            sibling = nodeX->parent->left;
                        }
                        if (sibling != nullptr) {
                            sibling->color = nodeX->parent->color;
                        }
                        if (nodeX->parent != nullptr) {
                            nodeX->parent->color = 0;
                            flipCount++;
                        }
                        if (sibling != nullptr && sibling->left != nullptr) {
                            sibling->left->color = 0;
                            flipCount++;
                        }
                        if (nodeX->parent != nullptr) {
                            rbTreeRightRotate(nodeX->parent);
                        }
                        nodeX = root;
                    }
                }
            }
        }

        if (nodeX != nullptr) {
            nodeX->color = 0;
            flipCount++;
        }
    }

    void deleteRBTreeNode(int bookID) {
        // Find the node to be deleted
        RBTreeNode* nodeToDelete = searchRBTree(bookID);

        // If the node is not found, return
        if (nodeToDelete == nullptr) {
            return;
        }

        // Identify the node to be used for rebalancing after deletion
        RBTreeNode* rebalanceNode;

        // Determine the node to be used for rebalancing and update rebalanceNode
        if (nodeToDelete->left == NULLNODE || nodeToDelete->right == NULLNODE) {
            rebalanceNode = nodeToDelete;
        } else {
            rebalanceNode = minimumRBTreeNode(nodeToDelete->right);
        }

        // Identify the child of rebalanceNode
        RBTreeNode* childOfRebalanceNode;
        if (rebalanceNode->left != NULLNODE) {
            childOfRebalanceNode = rebalanceNode->left;
        } else if (rebalanceNode->right != NULLNODE) {
            childOfRebalanceNode = rebalanceNode->right;
        } else {
            childOfRebalanceNode = NULLNODE;
        }
        // Perform the actual deletion
        if (childOfRebalanceNode != NULLNODE) {
            childOfRebalanceNode->parent = rebalanceNode->parent;
        }

        if (rebalanceNode->parent == nullptr) {
            // If the node to be deleted is the root
            root = childOfRebalanceNode;
        } else {
            if (rebalanceNode == rebalanceNode->parent->left) {
                rebalanceNode->parent->left = childOfRebalanceNode;
            } else {
                rebalanceNode->parent->right = childOfRebalanceNode;
            }
        }

        if (rebalanceNode != nodeToDelete) {
            // Copy data from the node to be deleted to the replacement node
            nodeToDelete->book = rebalanceNode->book;
        }

        if (rebalanceNode->color) {
            // If the replacement node is black, perform fix-up to maintain Red-Black Tree properties
            deleteRBTreeNodeFix(childOfRebalanceNode);
        }

        // Deallocate memory for the node to be deleted
        delete rebalanceNode;
    }

public:
    //constructor
    gatorLibrary() : root(nullptr), flipCount(0) {
        // Create a new node representing NULL in the Red-Black Tree
        NULLNODE = new RBTreeNode;
        NULLNODE->book.bookId = 0;
        NULLNODE->book.bookName = "";
        NULLNODE->book.authorName = "";
        NULLNODE->book.availabilityStatus = false;
        NULLNODE->book.borrowedBy = -1;
        NULLNODE->color = 0; // Assuming black as the default color
        NULLNODE->left = nullptr;
        NULLNODE->right = nullptr;
        NULLNODE->parent = nullptr;

        root = NULLNODE;

        // Open a file for writing output
        // outputFile.open(outputFileName);
    }

    void getColorFlipCount() {
        outputFile << "Color Flip Count: " << flipCount << endl << endl;
        return;
    }

    RBTreeNode* searchRBTree(int bookId) {
        // Call the helper function searchRBTreeHelper to traverse the tree recursively.
        RBTreeNode* result = searchRBTreeHelper(this->root, bookId);
        if (result == NULLNODE) {
            return nullptr; 
        }

        return result;
    }

    void findClosestBook(int targetId) {
        int minDiff = INT_MAX;
        vector<Book> listOfBooks;
        inOrder(root, listOfBooks);

        vector<Book> closestBooks;

        for (const Book& book : listOfBooks) {
            int diff = abs(targetId - book.bookId);
            if (minDiff > diff) {
                minDiff = diff;
                closestBooks = {book};
            } else if (minDiff == diff) {
                closestBooks.push_back(book);
            }
        }

        sort(closestBooks.begin(), closestBooks.end(), [](const Book& x, const Book& y) {
            return x.bookId < y.bookId;
        });

        for (const Book& book : closestBooks) {
            printBook(book.bookId);
        }
    }

    void printBookHelper(RBTreeNode* node) {
        outputFile << "BookID: " << node->book.bookId << endl;
        outputFile << "Title: " << "\"" << node->book.bookName << "\"" <<  endl;
        outputFile << "Author: " << "\"" << node->book.authorName << "\"" << endl;
        outputFile << "Availability: " << "\"" <<(node->book.availabilityStatus ? "Yes" : "No") << "\"" << endl;
        outputFile << "BorrowedBy: " << (node->book.borrowedBy == -1 ? "None" : std::to_string(node->book.borrowedBy)) << endl;
        // Print reservation details
        outputFile << "Reservations: [";
        for (auto it = node->book.reservationHeap.begin(); it != node->book.reservationHeap.end(); ++it) {
            outputFile << it->patronId;
            if (std::next(it) != node->book.reservationHeap.end()) {
                outputFile << ", ";
            }
        }
        outputFile << "]" << endl << endl;
    }
    void printBook(int bookID) {
        RBTreeNode* node = searchRBTree(bookID);
        if (node) {
            printBookHelper(node);
        } else {
            outputFile << "Book " << bookID << " not found in the Library" <<endl<<endl;
            return;
        }
    }
    void printBooks(int bookID1, int bookID2) {
       for(int i=bookID1;i<=bookID2;i++) {
            RBTreeNode* node = searchRBTree(i);
            if(node){
                printBookHelper(node);
            }
       }
    }

    void deleteBook(int bookId) {
        RBTreeNode* node = searchRBTree(bookId);
        vector<ReservationNode> reservations;  // Correct type for storing ReservationNode objects
        for (auto it = node->book.reservationHeap.begin(); it != node->book.reservationHeap.end(); ++it) {
            reservations.push_back(*it);
        }

        if (!reservations.empty()) {
            outputFile << "Book " << bookId << " is no longer available. ";

            if (reservations.size() > 1) {
                // Book is reserved by multiple patrons
                outputFile << "Reservations made by Patrons: ";
                for (const auto& reservation : reservations) {
                    outputFile <<reservation.patronId << " ";
                }
                outputFile << "have been cancelled!" << endl<< endl;
            } else if (reservations.size() == 1) {
                // Book is reserved by one patron
                outputFile << "Reservation made by Patron " << reservations[0].patronId << " has been cancelled!" << endl << endl;
            }
        } else {
            outputFile << "Book " << bookId << " is no longer available. "<<endl<<endl;
        }
        // Remove the book and its reservations
        deleteRBTreeNode(bookId);
    }

    void insertBook(int bookID, const string& bookName, const string& authorName, bool availabilityStatus, int borrowedBy, vector<ReservationNode>& reservationHeap) {
        if (searchRBTree(bookID) != nullptr) {
            outputFile << "Error: Duplicate Book ID " << bookID << ". Book not inserted." << endl << endl; // Book with the bookID does exists, handle the error print a message or throw an exception
            return;
        }
        RBTreeNode* newNode = new RBTreeNode;
        newNode->book.bookId = bookID;
        newNode->book.bookName = bookName;
        newNode->book.authorName = authorName;
        newNode->book.availabilityStatus = availabilityStatus;
        newNode->book.borrowedBy = borrowedBy;
        newNode->book.reservationHeap = reservationHeap;
        newNode->left = NULLNODE;
        newNode->right = NULLNODE;
        newNode->color = 1; 
        if (root == NULLNODE) { // If the root is NULLNODE (initially empty tree), set the new node as the root
            root = newNode;
            newNode->color = 0; // Set the color of the root node to black
            return;
        }
        RBTreeNode* parent = nullptr; // Traverse to find the appropriate position to insert the new node
        RBTreeNode* current = root;
        while (current != NULLNODE) {
            parent = current;
            if (newNode->book.bookId < current->book.bookId) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        newNode->parent = parent; // Set the parent for the new node based on the traversal
        if (newNode->book.bookId < parent->book.bookId) { // Assign the new node as the appropriate child of its parent
            parent->left = newNode;
        } else {
            parent->right = newNode;
        }
        insertRBTreeNodeFix(newNode); // Function to fix up the tree (if needed)
    }

    void borrowBook(int patronID, int bookID, int patronPriority) {
        RBTreeNode* bookNode = searchRBTree(bookID);

        if (bookNode) {
            if (bookNode->book.availabilityStatus) {
                // Book is available, update status
                bookNode->book.availabilityStatus = false;
                bookNode->book.borrowedBy = patronID;
                outputFile << "Book " << bookID << " Borrowed by Patron " << patronID << endl << endl;
            } else {
                // Book is not available, add to reservation heap
                bookNode->book.reservationHeap.emplace_back(patronID, patronPriority, getCurrentTimestamp());
                // Assuming you have a function to heapify up in the reservation heap
                heapifyUp(bookNode->book.reservationHeap, bookNode->book.reservationHeap.size() - 1);
                outputFile << "Book " << bookID << " Reserved by Patron " << patronID << endl << endl;
            }
        } else {
            outputFile << "Book " << bookID << " not found in the Library" << endl << endl;
        }
    }

    void returnBook(int patronID, int bookID) {
        RBTreeNode* bookNode = searchRBTree(bookID);

        if (bookNode) {
            if(bookNode->book.borrowedBy == patronID) {
                bookNode->book.availabilityStatus = true;
                bookNode->book.borrowedBy = -1; // Assuming -1 represents no patron
                outputFile << "Book " << bookID << " Returned by Patron " << patronID << endl << endl;
                if (!bookNode->book.reservationHeap.empty()) {
                    // Book has reservations, assign to the highest priority patron
                    auto highestPriorityReservation = bookNode->book.reservationHeap.front();
                    bookNode->book.reservationHeap.erase(bookNode->book.reservationHeap.begin());
                    bookNode->book.availabilityStatus = false;
                    bookNode->book.borrowedBy = highestPriorityReservation.patronId;

                    outputFile << "Book " << bookID << " Allotted to Patron " << highestPriorityReservation.patronId << endl << endl;
                }
            } else {
                outputFile << "Patron " << patronID << " did not borrow Book " << bookID << endl << endl;
            }
        } else {
            outputFile << "Book " << bookID << " not found in the Library" << endl << endl;
        }
    }

    RBTreeNode* findClosestRBTreeNode(int targetID) {
        RBTreeNode* closest = nullptr;
        int minDiff = INT_MAX;

        while (root != nullptr) {
            int diff = abs(root->book.bookId - targetID);

            if (diff < minDiff) {
                minDiff = diff;
                closest = root;
            }

            if (root->book.bookId == targetID) {
                // Found an exact match, no need to search further
                return root;
            } else if (root->book.bookId < targetID) {
                root = root->right;
            } else {
                root = root->left;
            }
        }

        return closest;
    }

    void controlGatorTaxi(string inputFileName) {
        setOutputFileName(inputFileName);
        openOutputFile();
        // Open the input file and read it line by line
        ifstream inputFile(inputFileName);
        string eachLine;
        while (getline(inputFile, eachLine)) {
            // Use string stream to extract relevant information from each line
            stringstream ss(eachLine);

            // If the line contains "Insert", extract the ride information and call Insert function
            if(eachLine.find("Quit()") != string::npos) {
                outputFile << "Program Terminated!!" << endl << endl;
                break;  
            }
            else if (eachLine.find("InsertBook") != string::npos) {
                int bookId, borrowedBy=-1;
                vector<ReservationNode> reservationHeap;
                string bookName,authorName, availabilityStatus;
                char ch;
                string skip;

                ss.ignore(11);
                // ss >> bookId >> ch >> bookName >> ch >> authorName >> ch >> (availabilityStatus);
                ss >> bookId >> ch;
                // Read title until the next '"' and ignore the '"'
                getline(getline(ss, skip, '"'), bookName, '"'); 
                getline(getline(ss, skip, '"'), authorName, '"');
                getline(getline(ss, skip, '"'), availabilityStatus, '"');  

                bool availability = availabilityStatus == "Yes" ? true : false;
                if(ss.peek() == ',') {
                    ss.ignore(1,',');
                    ss >> borrowedBy;
                }
                insertBook(bookId,bookName,authorName,availability,borrowedBy, reservationHeap);
            }
            // If the line contains "Print", extract the ride information and call Print function
            else if (eachLine.find("PrintBook(") != string::npos) {
                int bookId1;
                ss.ignore(10);
                ss >> bookId1;
                printBook(bookId1);
            } else if (eachLine.find("PrintBooks(") != string::npos) {
                int bookId1, bookId2 = -1;
                ss.ignore(11);
                ss >> bookId1;

                if (ss.peek() == ',') {
                    ss.ignore(1, ',');
                    ss >> bookId2;
                    printBooks(bookId1, bookId2);
                }
            } else if (eachLine.find("BorrowBook") != string::npos) {
                int patronID, bookID, patronPriority;
                char ch;
                ss.ignore(11);  // Adjusted to skip "BorrowBook("
                ss >> patronID >> ch >> bookID >> ch >> patronPriority;

                borrowBook(patronID, bookID, patronPriority);
            } else if (eachLine.find("ReturnBook") != string::npos) {
                int patronID, bookID;
                char ch;
                ss.ignore(11);  // Adjusted to skip "ReturnBook("
                ss >> patronID >> ch >> bookID;

                returnBook(patronID, bookID);
            } else if (eachLine.find("DeleteBook") != string::npos) {
                int bookID;
                ss.ignore(11);  // Adjusted to skip "DeleteBook("
                ss >> bookID;

                //function to delete book with given bookID
                deleteBook(bookID);
            } else if (eachLine.find("ColorFlipCount") != string::npos) {
                getColorFlipCount();
            } else if (eachLine.find("FindClosestBook") != string::npos) {
                int bookID;
                ss.ignore(16);
                ss >> bookID;
                findClosestBook(bookID);
            }
        }
        inputFile.close();
        closeOutputFile();
    }
};

// The main function of the program
int main(int argc, char * argv[]) {
    // Create an instance of gatorLibrary object
    gatorLibrary gt;
    //Retrieve the input file name from the command line arguments 
    string inputFileName = argv[1];
    // Execute the controlGatorTaxi method of gatorLibrary with the input file name
    gt.controlGatorTaxi(inputFileName);
    // Terminate the program with a success status code
    return 0;
}