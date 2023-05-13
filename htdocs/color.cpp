#include <iostream>
#include <unistd.h>

using namespace std;
int main()
{
    char *data;
    char *length;
    char color[20];
    char c = 0;
    int flag = -1;

    cout << "Content-Type: text/html\r\n" << endl;
    cout << "<HTML>\n<TITLE>CGI</TITLE>"
              << "<BODY>\n<H1>Good Job!</H1>"
              << "<H2>You successfully run the server and a CGI script.</H2>"
              << "<P>The background color is: ";

    if ((data = getenv("QUERY_STRING")) != NULL)
    {
        while (*data != '=')
        {
            data++;
        }
        data++;
        sprintf(color, "%s", data);
    }
    if ((length = getenv("CONTENT_LENGTH")) != NULL)
    {
        int i;
        for (i = 0; i < atoi(length); i++)
        {
            read(STDIN_FILENO, &c, 1);
            if (c == '=')
            {
                flag = 0;
                continue;
            }
            if (flag > -1)
            {
                color[flag++] = c;
            }
        }
        color[flag] = '\0';
    }
    std::cout << color << std::endl;
    std::cout << "<BODY style=\"background-color:" << color << "\">";
    std::cout << "</BODY>\n</HTML>" << std::endl;
    return 0;
}