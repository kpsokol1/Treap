using namespace std;
#include <time.h>  //used to set random seed for priority
#include <string>   //needed for windows?
template <class keytype>
struct Node{
    keytype key;
    float priority;
    Node *left;
    Node *right;
    Node *parent;
    int leftSubTreeSize;
    int rightSubTreeSize;

    explicit Node(keytype k, float p){
        key = k;
        priority = p;
        leftSubTreeSize = 0;
        rightSubTreeSize = 0;
        left = nullptr;
        right = nullptr;
        parent = nullptr;
    }
    explicit Node(Node *node) {
        key = node->key;
        priority = node->priority;
        leftSubTreeSize = node->leftSubTreeSize;
        rightSubTreeSize = node->rightSubTreeSize;
        left = node->left;
        right = node->right;
        parent = node->parent;
    }
};

template <class keytype>
class Treap {
private:
    Node<keytype> *leafNodeInserted;    //keeps track of node inserted before it is rotated up the tree to fix the priority


    int mSize;

    Node<keytype>* recursiveInsert(Node<keytype>* node, Node<keytype>* parent, keytype k, float p);

    void recursivePreorder(Node<keytype>* node);

    void recursiveInorder(Node<keytype>* node);

    void recursivePostorder(Node<keytype>* node);

    Node<keytype>* recursivePredecessor(Node<keytype>* root, Node<keytype>*& predecessor, keytype k); //fixme are we really returning

    Node<keytype>* recursiveSuccessor(Node<keytype>* root, Node<keytype>*& successor, keytype k);   //fixme are we really returning

    void rotateRight(Node<keytype>* node);

    void rotateLeft(Node<keytype>* node);

    void fixPrioritiesAfterInsert(Node<keytype>* node);

    keytype recursiveSelect (int pos, int originalPos, Node<keytype> *root, Node<keytype> *parent, Node<keytype> *originalParent); //fixme just for testing

    Node<keytype>* recursiveInorderCopy(Node<keytype>* node, Node<keytype>* parent);

    void emptyTree(Node<keytype>* node);

    int recursiveRank (int pos, Node<keytype> *root, keytype k);

public:
    Node<keytype> *root; //fixme testing

    Node<keytype>* recursiveSearch(Node<keytype>* root, keytype k); //fixme public for testing

    Treap();

    Treap(keytype k[], float p[], int s);

    ~Treap();

    Treap& operator=(const Treap& other);

    Treap(const Treap &oldTreap);

    void insert(keytype k);

    void insert(keytype k, float p);

    int remove (keytype k);

    keytype select (int pos);

    int rank(keytype k);

    void preorder();

    void inorder();

    void postorder();

    keytype successor(keytype k);

    keytype predecessor(keytype k);

    float search(keytype k);

    int size(){
        return mSize;
    }
};

//Constructors

//default constructor
template <class keytype>
Treap<keytype>::Treap(){
    srand(time(NULL)); //set random seed
    root = nullptr;     //empty tree so no root node, just a null pointer
    leafNodeInserted = nullptr;
    mSize = 0;
}

//advanced constructor
template <class keytype>
Treap<keytype>::Treap(keytype k[], float p[], int s){
    srand(time(NULL)); //set random seed
    root = nullptr;
    leafNodeInserted = nullptr;
    mSize = 0;
    for(int i = 0; i < s; i++){     //iterate through each key and priority and insert it into the Treap
        insert(k[i], p[i]);
    }
}

//destructor
template <class keytype>
Treap<keytype>::~Treap(){
    emptyTree(root);    //fixme is this right?
}

//copy assignment operator (deep copy)
template <class keytype>
Treap<keytype>& Treap<keytype>::operator=(const Treap<keytype> &other){
    root = nullptr; //fixme does recursion work with root being nullptr? and do the run times work with insert
    leafNodeInserted = nullptr;

    //recursiveInorderInsert(oldTreap.root); //just build the new tree by inserting each new node by an inorder traversal of the old tree (traversal order doesn't matter, same tree will result)
    recursiveInorderCopy(other.root, nullptr); //just build the new tree by inserting each new node by an inorder traversal of the old tree (traversal order doesn't matter, same tree will result)
    mSize = other.mSize; //update the mSize of the new tree

    return *this;
}

//copy constructor (deep copy)
template <class keytype>
Treap<keytype>::Treap(const Treap<keytype> &oldTreap){
    root = nullptr; //fixme does recursion work with root being nullptr? and do the run times work with insert
    leafNodeInserted = nullptr;

    //recursiveInorderInsert(oldTreap.root); //just build the new tree by inserting each new node by an inorder traversal of the old tree (traversal order doesn't matter, same tree will result)
    recursiveInorderCopy(oldTreap.root, nullptr); //just build the new tree by inserting each new node by an inorder traversal of the old tree (traversal order doesn't matter, same tree will result)
    mSize = oldTreap.mSize; //update the mSize of the new tree
}

//builds a new tree by doing an inorder traversal of the old tree
template <class keytype>
Node<keytype>* Treap<keytype>::recursiveInorderCopy(Node<keytype>* node, Node<keytype>* parent) {
    //*Note an inorder traversal visits left subtree -> currentNode -> right subtree
    if (node == nullptr) {
        return nullptr;
    }
    //just reconstruct the tree again
    Node<keytype> *newNode = new Node<keytype>(node);   //create a new node
    if (node->parent == nullptr) {
        root = newNode;
    }
    newNode->parent = parent;
    newNode->left = recursiveInorderCopy(node->left, newNode);
    newNode->right = recursiveInorderCopy(node->right,newNode);
    return newNode;
}
//completely deletes all contents from the Treap
template <class keytype>
void Treap<keytype>::emptyTree(Node<keytype>* node){
    if(node == nullptr){
        return;
    }
    emptyTree(node->left);
    emptyTree(node->right);
    delete node;    //fixme is this right?
    node = nullptr;
}

//inserts a node with a random priority
template <class keytype>
void Treap<keytype>::insert(keytype k){
    float priority = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); //generate random priority between 0 and 1
    insert(k,priority); //insert the element
}

//inserts a node with a set priority
template <class keytype>
void Treap<keytype>::insert(keytype k, float p){
    //Steps:
    //      1. Do regular binary tree insert
    //      2. Rotate the node up the tree until its priority is greater than its parent

    //Step 1:----------------------
    //if there is nothing in the tree, then make a root node for the root pointer to point to
    //cout << k << "   " << p << endl;
    if(root == nullptr){
        Node<keytype> *node = new Node<keytype>(k,p);   //create a new node
        root = node;                                    //set the root pointer to our new node
        root->parent = nullptr;                         //set the root's parent pointer to null since root has no parent
        mSize++;                                         //increment mSize of list
        return; //don't need to fix priorities
    }
        //tree is not empty
    else{
        if(k > root->key){                                              //the node we want to insert has a greater key than its parent so we recurse on the parent's right subtree
            root->rightSubTreeSize++;                                   //increment the mSize of the right subtree
            root->right = recursiveInsert(root->right, root, k, p);     //recursively insert on right subtree
        }
        else{                                                           //the node to insert is less than its parent
            root->leftSubTreeSize++;                                    //increment mSize of left subtree
            root->left = recursiveInsert(root->left, root, k, p);       //recursively insert on left subtree
        }
    }
    //Step 2:----------------------
    //fix priority
    fixPrioritiesAfterInsert(leafNodeInserted);
    mSize++;
}

//recursively insert function. Same functionality as regular insert but takes the required parameters for recursion
template <class keytype>
Node<keytype>* Treap<keytype>::recursiveInsert(Node<keytype>* node, Node<keytype>* parent, keytype k, float p){
    //if there is nothing in the tree, then make a root node for the root pointer to point to
    if(node == nullptr){
        Node<keytype> *node = new Node<keytype>(k,p);   //create a new node
        node->parent = parent;                          //we have passed a leaf so set the parent of the node to insert to its parent leaf
        leafNodeInserted = node;                        //keep track of the node we inserted to we can know what node to rotate up later
        return node;
    }
        //tree is not empty
    else{
        if(k > node->key){                                              //the node we want to insert has a greater key than its parent so we recurse on the parent's right subtree
            node->rightSubTreeSize++;                                   //increment the mSize of the right subtree
            node->right = recursiveInsert(node->right, node, k, p);     //recursively insert on right subtree
        }
        else{                                                           //the node to insert is less than its parent
            node->leftSubTreeSize++;                                    //increment mSize of left subtree
            node->left = recursiveInsert(node->left, node, k, p);       //recursively insert on left subtree
        }
    }
    return node;
}

//wrapper method for recursive preorder
//Preorder: Center -> Left -> Right
template <class keytype>
void Treap<keytype>::preorder(){
    recursivePreorder(root);
    cout << endl;   //add a newline after we print the order out
}

//preorder traversal of the tree
//Preorder: Center -> Left -> Right
template <class keytype>
void Treap<keytype>::recursivePreorder(Node<keytype>* node){
    if (node == nullptr){               //base case, we have recursed to an empty node
        return;
    }
    cout << node->key << " ";           //visit center
    recursivePreorder(node->left);      //visit left subtree
    recursivePreorder(node->right);     //visit right subtree
}

//wrapper method for recursive postorder
//Postorder: Left -> Right -> Center
template <class keytype>
void Treap<keytype>::postorder(){
    recursivePostorder(root);
    cout << endl;       //add a newline after we print the order out
}

//postorder traversal of the tree
//Postorder: Left -> Right -> Center
template <class keytype>
void Treap<keytype>::recursivePostorder(Node<keytype>* node){
    if (node == nullptr){               //base case, we have recursed to an empty node
        return;
    }
    recursivePostorder(node->left);      //visit left subtree
    recursivePostorder(node->right);     //visit right subtree
    cout << node->key << " ";           //visit center

}

//wrapper method for recursive inorder
template <class keytype>
void Treap<keytype>::inorder(){
    recursiveInorder(root);
    cout << endl;           //add a newline after we print the order out
}

template <class keytype>
void Treap<keytype>::recursiveInorder(Node<keytype>* node){
    if (node == nullptr){               //base case, we have recursed to an empty node
        return;
    }

    recursiveInorder(node->left);       //visit left subtree
    cout << node->key << " ";           //visit center
    recursiveInorder(node->right);      //visit right subtree
}

//wrapper method for recursiveSuccessor
//returns successor of node in tree
template <class keytype>
keytype Treap<keytype>::successor(keytype k){
    Node<keytype> *successor = nullptr;     //create an empty node that recursiveSuccessor will save the successor to
    recursiveSuccessor(root,successor,k);

    //return k if no successor
    if(successor == nullptr){
        return k;
    }

        //return the value at the successor
    else{
        return successor->key;
    }
}

//wrapper method for recursivePredecessor
//returns predecessor of node in tree
template <class keytype>
keytype Treap<keytype>::predecessor(keytype k){
    Node<keytype> *predecessor = nullptr;       //create an empty node that recursiveSuccessor will save the successor to
    recursivePredecessor(root,predecessor,k);

    //return k if no predecessor
    if(predecessor == nullptr){
        return k;
    }

        //return the value at the predecessor
    else{
        return predecessor->key;
    }
}

//recursively returns the predescessor of a given node in the tree
template <class keytype>
Node<keytype>* Treap<keytype>::recursivePredecessor(Node<keytype>* root, Node<keytype>*& predecessor, keytype k){

    //base case (at the leftmost leaf or the tree is empty)
    if(root == nullptr){
        predecessor = nullptr;
        return predecessor;
    }

    //if we are standing on the key...
    if(root->key == k){
        if(root->left != nullptr){              //find the largest element in the left subtree
            Node<keytype> *node = root->left;   //go to left subtree
            while(node->right != nullptr){      //find largest element in left subtree by going to the right
                node = node->right;             //go to the right
            }
            predecessor = node;                 //set the predecessor once we have gone as far right as possible
        }
    }

        //recurse down the left subtree if the key is smaller than the root
    else if(k < root->key){
        recursivePredecessor(root->left, predecessor, k);
    }

        //recurse down the right subtree if key is greater than the root, save the predecessor as the root because if we hit a leaf that will be the predecessor
    else{
        predecessor = root;
        recursivePredecessor(root->right, predecessor, k);
    }
}

template <class keytype>
Node<keytype>* Treap<keytype>::recursiveSuccessor(Node<keytype>* root, Node<keytype>*& successor, keytype k){

    //base case (at the rightmost leaf or the tree is empty)
    if(root == nullptr){
        successor = nullptr;
        return successor;
    }

    //if we are standing on the key...
    if(root->key == k){
        if(root->right != nullptr){             //find the smallest element in the right subtree
            Node<keytype> *node = root->right;  //go to right subtree
            while(node->left != nullptr){       //find the largest element in the left subtree by going to the right
                node = node->left;              //go to the left
            }
            successor = node;                   //set the sucessor once we have gone as far left as possible
        }
    }

        //recurse down the left subtree if key is less than the root, save the successor as the root because if we hit a leaf that will be the successor
    else if(k < root->key){
        successor = root;
        recursiveSuccessor(root->left, successor, k);
    }

        //recurse down the right subtree if the key is greater than the root
    else{
        recursiveSuccessor(root->right, successor, k);
    }
}

//wrapper method for recursive search
template <class keytype>
float Treap<keytype>::search(keytype k){
    Node<keytype> *result = recursiveSearch(root,k);        //return the node in the tree with the given key
    if(result != nullptr){                                  //if the node exists with the given key then return its priority
        return result->priority;
    }
    return -1;                                              //the node with the given key could not be found
}

//recursively searches the tree for a given key
template <class keytype>
Node<keytype>* Treap<keytype>::recursiveSearch(Node<keytype>* root, keytype k){
    if(root == nullptr){                        //base case, we have gone through the whole tree and didn't find the node
        return nullptr;
    }
    else if(root->key == k){                    //found the node
        return root;
    }
    else if(k < root->key){                     //the key we are looking for is less than the key for the node we are looking at
        return recursiveSearch(root->left, k);  //search recursively on the left subtree
    }
    else{                                       //the key we are looking for is greater than the key for the node we are looking at
        return recursiveSearch(root->right, k); //serach recursively on the right subtree
    }
}

//rotates a node to the right
template <class keytype>
void Treap<keytype>::rotateRight(Node<keytype>* node){
    Node<keytype> *rightChildOfNode = node->right;          //keep track of the right child (if any) of the node to be rotated right

    node->right = node->parent;                             //the node's original parent is now its right child
    node->right->left = rightChildOfNode;                   //the new right node takes the original node's right child as its left child

    //fix parent pointers
    node->parent = node->parent->parent;                    //the original node is rotated up so its parent becomes the original node's grandparent
    if(node->parent == nullptr){
        root = node;
    }
    node->right->parent = node;                             //the new right node now has the original node as its parent

    if(node->right->left != nullptr){                       //rightChildOfNode may have been null so don't set parent if a nullptr
        node->right->left->parent = node->right;            //the new right node is now the parent of the original nodes's right child. This node will be a left child of the new right node
    }

    if(node->parent != nullptr){                            //check to make sure we are not at the top of the tree
        //determine if the node is a left or right child
        if(node->key < node->parent->key){                  // node is left child so set the parent's left child to the current node
            node->parent->left = node;
        }
        else{                                               //node is right child so set the parent's right child to the current node
            node->parent->right = node;
        }
    }

    //fix subtree counts
    if(rightChildOfNode != nullptr){       //if the original node had a right subtree then the new node right has a left subtree the mSize of the rightChildOfNode tree;
        node->right->leftSubTreeSize = rightChildOfNode->leftSubTreeSize + rightChildOfNode->rightSubTreeSize + 1;
    }
    else{                                   //new right node has no left subtree so its mSize is 0
        node->right->leftSubTreeSize = 0;
    }

    node->rightSubTreeSize = node->right->leftSubTreeSize + node->right->rightSubTreeSize + 1;  //the node's right subtree mSize is set to the mSize of its right node tree mSize
}

//rotates a node to the left
template <class keytype>
void Treap<keytype>::rotateLeft(Node<keytype>* node){
    Node<keytype> *leftChildOfNode = node->left;        //keep track of the left child (if any) of the node to be rotated left


    node->left = node->parent;                          //the node's original parent is now its left child
    node->left->right = leftChildOfNode;                //the new left node takes the original node's left child as its right child

//fix parent pointers
    node->parent = node->parent->parent;                //the original node is rotated up so its parent becomes the original node's grandparent
    if(node->parent == nullptr){
        root = node;
    }
    node->left->parent = node;                          //the new left node now has the original node as its parent

    if(node->left->right != nullptr){                   //leftChildOfNode may have been null so don't set parent if a nullptr
        node->left->right->parent = node->left;         ////the new left node is now the parent of the original nodes's left child. This node will be a right child of the new left node
    }

    if(node->parent != nullptr){                        //check to make sure we are not at the top of the tree
        if(node->key < node->parent->key){              // node is left child so set the parent's left child to the current node
            node->parent->left = node;
        }
        else{
            node->parent->right = node;                 //node is right child so set the parent's right child to the current node
        }
    }

    //fix subtree counts
    if(leftChildOfNode != nullptr){     //if the original node had a left subtree then the new node left has a right subtree the mSize of the leftChildOfNode tree;
        node->left->rightSubTreeSize = leftChildOfNode->leftSubTreeSize + leftChildOfNode->rightSubTreeSize + 1;
    }
    else{                               //the node's right subtree mSize is set to the mSize of its right node tree mSize
        node->left->rightSubTreeSize = 0;
    }
    node->leftSubTreeSize = node->left->leftSubTreeSize + node->left->rightSubTreeSize + 1;     //the node's left subtree mSize is set to the mSize of its left node tree mSize
}

//iteratively rotates the inserted node up the tree after a regular binary search tree insert so the priorities are in the correct order
template <class keytype>
void Treap<keytype>::fixPrioritiesAfterInsert(Node<keytype>* node){
    //priority larger than parent leave it alone
    //*Note if equal priorities than the smaller key becomes the parent
    while(node->parent != nullptr && node->priority <= node->parent->priority){ //less than or equal to accounts for equal priorities
        //left child and priority smaller than parent
        if(node->key < node->parent->key){
            rotateRight(node);
        }
            //right child and priority larger than parent
        else{
            rotateLeft(node);
        }
    }

    //update root node pointer if the root node changed
    if(node->left == root || node->right == root){
        root = node;
    }
}

//deletes a node from the tree
template <class keytype>
int Treap<keytype>::remove (keytype k){ //fixme delete nodes
    //find node to remove
    Node<keytype> *nodeToRemove = recursiveSearch(root,k);
    if(nodeToRemove == nullptr){
        return 0;
    }

    //check how many children the node to delete has

    //if has no children delete it
    if(nodeToRemove->left == nullptr && nodeToRemove->right == nullptr){
        mSize--;                                                     //decrement the mSize of the tree
        //check if root
        if(nodeToRemove == root){
            delete root;
            root = nullptr;
            return 1;
        }
        if(nodeToRemove->key < nodeToRemove->parent->key){      //update parent's pointers accordingly
            nodeToRemove->parent->left = nullptr;               //deleted node was left child
        }
        else{
            nodeToRemove->parent->right = nullptr;              //deleted node was right child
        }
        Node<keytype> *counterNode = nodeToRemove;
        //update subtree counts
        while(counterNode->parent != nullptr){                  //go up the tree, starting from the predecessor and decrementing each nodes parent by 1 until we get to root
            if(counterNode->key < counterNode->parent->key){
                counterNode->parent->leftSubTreeSize--;         //decrement leftSubtree mSize of the node's parent
            }
            else{
                counterNode->parent->rightSubTreeSize--;        //decrement rightSubTree mSize of the node's parent
            }
            counterNode = counterNode->parent;                  //go up 1 in the tree
        }

        delete nodeToRemove;                                    //delete the node's contents
    }

        //if has one child replace by child //fixme is this the right logic for checking here?
    else if(nodeToRemove->left == nullptr || nodeToRemove->right == nullptr){
        mSize--;                                                     //decrement the mSize of the tree
        if(nodeToRemove == root){
            if(nodeToRemove->left == nullptr){
                root = nodeToRemove->right;
                root->parent = nullptr;
            }
            else{
                root = nodeToRemove->left;
                root->parent = nullptr;
            }
            //root = nodeToRemove->left == nullptr ? nodeToRemove->right : nodeToRemove->left;    //we know only 1 child is null so bubble up the child that is not null
        }
        else{
            if(nodeToRemove->key < nodeToRemove->parent->key){      //if node to remove is a left child replace parent's left child by the non-null child of the node removed
                if(nodeToRemove->left == nullptr){
                    nodeToRemove->right->parent = nodeToRemove->parent;
                    nodeToRemove->parent->left = nodeToRemove->right;
                }
                else{
                    nodeToRemove->left->parent = nodeToRemove->parent;
                    nodeToRemove->parent->left = nodeToRemove->left;
                }
                //nodeToRemove->parent->left = nodeToRemove->left == nullptr ? nodeToRemove->right : nodeToRemove->left;

            }
            else{//if node to remove is a right child replace parent's right child by the non-null child of the node removed
                if(nodeToRemove->left == nullptr){
                    nodeToRemove->right->parent = nodeToRemove->parent;
                    nodeToRemove->parent->right = nodeToRemove->right;
                }
                else {
                    nodeToRemove->left->parent = nodeToRemove->parent;
                    nodeToRemove->parent->right = nodeToRemove->left;

                    //nodeToRemove->parent->right = nodeToRemove->left == nullptr ? nodeToRemove->right : nodeToRemove->left;
                }
            }
        }
        Node<keytype> *counterNode = nodeToRemove;
        //update subtree counts
        while(counterNode->parent != nullptr){                  //go up the tree, starting from the predecessor and decrementing each nodes parent by 1 until we get to root
            if(counterNode->key < counterNode->parent->key){
                counterNode->parent->leftSubTreeSize--;         //decrement leftSubtree mSize of the node's parent
            }
            else{
                counterNode->parent->rightSubTreeSize--;        //decrement rightSubTree mSize of the node's parent
            }
            counterNode = counterNode->parent;                  //go up 1 in the tree
        }

        delete nodeToRemove;                                    //deletes the node's contents
    }

        //if has two children replace with predecessor and rotate up predecessor where necessary
    else{
        Node<keytype> *predecessor = nullptr;
        recursivePredecessor(root, predecessor, nodeToRemove->key); //predecessor saved to predecessor variable

        //set node trying to delete to the key and priority of the predecessor
        keytype pKey = predecessor->key;
        float pPriority = predecessor->priority;

        remove(predecessor->key);

        nodeToRemove->key = pKey;
        nodeToRemove->priority = pPriority;

        bool needsMoreBubbling = true;
        //bubble updated node down
        while(needsMoreBubbling){
            //left child and priority smaller than parent
            if(nodeToRemove->left != nullptr && nodeToRemove->right != nullptr){    //node to remove has two children
                if(nodeToRemove->left->priority < nodeToRemove->right->priority){
                    rotateRight(nodeToRemove->left);
                    //rotate smaller priority of two nodes up to not violate other priorities
                }
                else if(nodeToRemove->left->priority == nodeToRemove->right->priority){ //if equal priorities, rotate smaller key up
                    if(nodeToRemove->left->key < nodeToRemove->right->key){             //left node has smaller key
                        rotateRight(nodeToRemove->left);
                    }
                    else{                                                               //right node has smaller key
                        rotateLeft(nodeToRemove->right);
                    }
                }
                    //right child and priority larger than parent
                else{ //**
                    rotateLeft(nodeToRemove->right);
                }
            }
                //at this point we know that the node has 0 or 1 children
            else if(nodeToRemove->left != nullptr){         //node just has a left child
                rotateRight(nodeToRemove->left);
            }
            else if(nodeToRemove->right != nullptr){        //node just has a right child
                rotateLeft(nodeToRemove->right);
            }
            else{ //bubbled down to a leaf, no children
                break;
            }
            //check loop condition so not reading invalid memory
            if(nodeToRemove->left != nullptr && nodeToRemove->priority > nodeToRemove->left->priority){ //priority still not correct
                needsMoreBubbling = true;
            }
            else if(nodeToRemove->right != nullptr && nodeToRemove->priority > nodeToRemove->right->priority){  //priority still not correct
                needsMoreBubbling = true;
            }
            else{ //priorities correct
                needsMoreBubbling = false;
            }
        }
    }

    return 1;       //we found and removed the node
}

//wrapper function to return the element at the given position in an inorder list of the tree
template <class keytype>
keytype Treap<keytype>::select (int pos){
    return recursiveSelect(pos, pos, root, nullptr, root);
}

//wrapper function to return 1-based position of a given element in an inorder list of the tree
template <class keytype>
int Treap<keytype>::rank(keytype k){
    return recursiveRank(0, root, k);
}

//recursively works down the tree to find the element's rank
template <class keytype>
int Treap<keytype>::recursiveRank (int pos, Node<keytype> *root, keytype k){
    if(root == nullptr){                           //element doesnt exist
        return 0;
    }
    else if(root->key == k){                         //we found the element
        return pos + root->leftSubTreeSize + 1; //convert to 1-based
    }
    else if(k < root->key){                     //the key of the element we are looking for is less than the current node we are looking at
        return recursiveRank(pos, root->left, k);
    }
    else{                                       //the key of the element we are looking for is greater than the current node we are looking at
        pos += root->leftSubTreeSize + 1; //0-based, going to the right, we have skipped all indexes with keys less than what we are looking for
        return recursiveRank(pos, root->right, k);
    }
}

//recursively works down the tree to find the element with the given position
template <class keytype>
keytype Treap<keytype>::recursiveSelect (int pos, int originalPos, Node<keytype> *root, Node<keytype> *parent, Node<keytype> *originalParent){
    if(root->leftSubTreeSize + 1 == pos){                                   //we are standing on the element we are looking for
        return root->key;
    }
    else if(root->leftSubTreeSize < pos){                                       //the position we are looking for must be in the right sub tree
        return recursiveSelect(pos - (root->leftSubTreeSize+1), originalPos, root->right, root, originalParent);   //recursively search in the right subtree with the new position relative to the right subtree
    }
    else{                                                                       //the position we are looking for must be in the right left tree
        return recursiveSelect(pos, originalPos, root->left, root, originalParent);                                //recursively search in the left subtree with the new position relative to the left subtree
    }
}
