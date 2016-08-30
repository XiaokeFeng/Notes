/**
 * @file practise.cpp
 * @author fengxiaoke
 * @date 2016/08/08 15:50:36
 * @brief 
 *  
 **/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <queue>
#include <iostream>
#include <cmath>

typedef struct bintree_tag
{
    int data;
    struct bintree_tag* left;
    struct bintree_tag* right;
}bintree;

class Solution
{
public:
    /*
     * 计算一个矩形框包含的长方形个数
     */
    long long count_rectangles(int width, int height)
    {
        long long out = 0;
        for (int i = 1; i <= width; i++)
        {
            for (int j = 1; j <= height; j++)
            {
                out += i == j ? 0 : (width - i + 1) * (height - j + 1);
            }
        }
        return out;
    }    

    /*
     * ip转为u_long整数
     */
    uint32_t inet_addr(const char* cp)
    {
        if (NULL == cp)
        {
            return 0;
        }
        unsigned char ip[4] = {0};
        
        int i = 0; // for cp
        int j = 0; // for ip
        char temp[10];
        int last_i = i;
        while (i < strlen(cp) + 1)
        {
            if (cp[i] == '.' || cp[i] == '\0')
            {
                snprintf(temp, i - last_i + 1, "%s\n", cp + last_i);
                last_i = i + 1;
                unsigned char uc = (unsigned char)atoi(temp);
                ip[j++] = uc;
            }
            i++;
        }

        uint32_t out = 0;
        int m = 0;
        for (m = 3; m > 0; m--)
        {
            out |= ip[m];
            out <<= 8;
        }
        out |= ip[m];
        
        return out;
    }

    /*
     * 判断是否为完全二叉树
     * return :
     *          
     *          1   :   is complete bintree
     *          0   :   is not ...
     *          -3  :   left NULL && right NULL
     *          -4  :   left !NULL && right !NULL
     *          -1  :   left NULL && right !NULL
     *          -2  :   left !NULL && right NULL
     */
    // 深度遍历是不可行的
    // int first_child_case(bintree* root)
    // {
    //     if (root->left == NULL && root->right != NULL)
    //     {
    //         return -1;
    //     }
    //     if (root->left != NULL && root->right == NULL)
    //     {
    //         return -2;
    //     }
    //     if (root->left == NULL && root->right == NULL)
    //     {
    //         return -3;
    //     }
    //     if (root->left != NULL && root->right != NULL)
    //     {
    //         return -4;
    //     }
    // }
    // int is_complete_bintree(bintree* root)
    // {
    //     while (NULL != root)
    //     {
    //         int ret_first_child_case = first_child_case(root);
    //         std::cout << "process data:" << root->data << ", first child case:" << ret_first_child_case << std::endl;
    //         if (ret_first_child_case != -4)
    //         {
    //             return ret_first_child_case;
    //         }
    //         int ret_left = is_complete_bintree(root->left);
    //         int ret_right = is_complete_bintree(root->right);
    //         std::cout << "root:" << root->data << ", ret_left:" << ret_left << ", ret_right:" << ret_right << std::endl;
    //         // process child case
    //         if ((ret_left == -4 && ret_right != -1) || (ret_left == -3 && ret_right == -3) || (ret_left == -2 && ret_right == -3))
    //         {
    //             std::cout << "root:" << root->data << ", return 1" << std::endl;
    //             return 1;
    //         }
    //         // process ret
    //         else if (ret_left == 1 && ret_right != -1)
    //         {
    //             std::cout << "root:" << root->data << ", return 1" << std::endl;
    //             return 1;
    //         }
    //         else
    //         {
    //             return 0;
    //         }
    //     }
    //     // never reach this.
    //     return 1;
    // }
    
    int is_complete_bintree(bintree* root)
    {
        if (NULL != root)
        {
            _tree_queue.push(root);
        }

        bintree* node;
        while (!_tree_queue.empty())
        {
            node = _tree_queue.front();
            _tree_queue.pop();
            if (NULL == node)
            {
                while (!_tree_queue.empty())
                {
                    node = _tree_queue.front();
                    if (NULL != node)
                    {
                        return 0;
                    }
                    _tree_queue.pop();
                }
                return 1;
            }
            _tree_queue.push(node->left);
            _tree_queue.push(node->right);
        }
    }

    /*
     * 最小全集串，一个字符串由1~5的数字组成，找出包含1~5的、且长度最小的串
     */
    int min_complete_str(const char* src, int* start, int* end)
    {
        if (NULL == src)
        {
            *start = 0;
            *end = 0;
            return -1;
        }

        for (int i = 0; i < strlen(src); i++)
        {

        }
    }

private:
    std::queue<bintree*> _tree_queue;

};

int main()
{
    Solution s;
    std::cout << "count_rectangles(3, 3) = " << s.count_rectangles(3, 3) << std::endl;
    std::cout << "inet_addr(\"127.0.0.1\") = " << s.inet_addr("127.0.0.1") << std::endl;
    std::cout << "inet_addr(\"255.255.255.1\") = " << s.inet_addr("255.255.255.1") << std::endl;

    // level 0
    bintree head;
    // level 1
    bintree right1;
    bintree left1;
    head.right = &right1;
    head.left = &left1;
    head.data = 0;
    // level 2
    bintree left11;
    bintree left12;
    bintree right11;
    left1.left = NULL;
    // left1.left = &left11;
    // left1.right = NULL;
    left1.right = &left12;
    left1.data = 1;
    // right1.left = &right11;
    right1.left = NULL;
    right1.right = NULL;
    right1.data = 2;
    // add null leaves
    left12.right = left12.left = NULL;
    left12.data = 4;
    left11.right = left11.left = NULL;
    left11.data = 3;
    right11.right = right11.left = NULL;
    right11.data = 5;
    std::cout << "is_complete_bintree(&head) = " << s.is_complete_bintree(&head) << std::endl;
   
    
    return 0;
}





















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
