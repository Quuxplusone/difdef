cat >a <<EOF
/*
   Difference
   inside
   a comment
*/
int main()
{
    puts("/*");
    puts("   Difference");
    puts("   not inside");
    puts("   a comment");
    puts("*/");
}
EOF

cat >b <<EOF
/*
   Difference
   within
   a comment
*/
int main()
{
    puts("/*");
    puts("   Difference");
    puts("   not within");
    puts("   a comment");
    puts("*/");
}
EOF

./difdef -DA a -DB b >out
cat >expected <<EOF
#ifdef A
/*
   Difference
   inside
   a comment
*/
#endif /* A */
#ifdef B
/*
   Difference
   within
   a comment
*/
#endif /* B */
int main()
{
    puts("/*");
    puts("   Difference");
#ifdef A
    puts("   not inside");
#endif /* A */
#ifdef B
    puts("   not within");
#endif /* B */
    puts("   a comment");
    puts("*/");
}
EOF
diff expected out

#rm -f a b expected out
