void createLocalA(Matrix &A,mesh m){
    float u_bar = m.getParameter(ADJECTIVE_VELOCITY);
    A.at(0).at(0) += -u_bar/2;  A.at(0).at(1) += u_bar/2;
    A.at(1).at(0) += -u_bar/2;  A.at(1).at(1) += u_bar/2;
}

void createLocalB(Matrix &B,mesh m){
    float l = m.getParameter(ELEMENT_LENGTH);
    float nu = m.getParameter(DYNAMIC_VISCOSITY);
    B.at(0).at(0) += nu/l;      B.at(0).at(1) += -nu/l;
    B.at(1).at(0) += -nu/l;     B.at(1).at(1) += nu/l;
}

void createLocalC(Matrix &C,mesh m){
    float rho = m.getParameter(DENSITY);
    C.at(0).at(0) += -1/(2*rho);    C.at(0).at(1) += 1/(2*rho);
    C.at(1).at(0) += -1/(2*rho);    C.at(1).at(1) += 1/(2*rho);
}

void createLocalD(Matrix &D,mesh m){
    D.at(0).at(0) += -0.5;  D.at(0).at(1) += 0.5;
    D.at(1).at(0) += -0.5;  D.at(1).at(1) += 0.5;
}

Matrix createLocalK(int element,mesh &m){
    Matrix K,A,B,C,D;

    zeroes(A,2);
    zeroes(B,2);
    zeroes(C,2);
    zeroes(D,2);
    createLocalA(A,m);
    createLocalB(B,m);
    createLocalC(C,m);
    createLocalD(D,m);

    Vector row1, row2, row3, row4;


    row1.push_back(A.at(0).at(0)+B.at(0).at(0)); 
    row1.push_back(A.at(0).at(1)+B.at(0).at(1));
    row1.push_back(C.at(0).at(0));                  
    row1.push_back(C.at(0).at(1));

    row2.push_back(A.at(1).at(0)+B.at(1).at(0)); 
    row2.push_back(A.at(1).at(1)+B.at(1).at(1));
    row2.push_back(C.at(1).at(0)); 
    row2.push_back(C.at(1).at(1));

    row3.push_back(D.at(0).at(0)); 
    row3.push_back(D.at(0).at(1));
    row3.push_back(0); 
    row3.push_back(0);

    row4.push_back(D.at(1).at(0)); 
    row4.push_back(D.at(1).at(1));
    row4.push_back(0); 
    row4.push_back(0);

    K.push_back(row1); 
    K.push_back(row2); 
    K.push_back(row3); 
    K.push_back(row4);

    return K;
}

Vector createLocalb(int element,mesh &m){
    Vector b;

    float f = m.getParameter(EXTERNAL_FORCE), l = m.getParameter(ELEMENT_LENGTH);
    
    b.push_back(f*l/2); 
    b.push_back(f*l/2); 
    b.push_back(0); 
    b.push_back(0);

    return b;
}

void crearSistemasLocales(mesh &m,vector<Matrix> &localKs,vector<Vector> &localbs){
    for(int i=0;i<m.getSize(ELEMENTS);i++){
        localKs.push_back(createLocalK(i,m));
        localbs.push_back(createLocalb(i,m));
    }
}

void assemblyK(element e,Matrix localK,Matrix &K,int nnodes){
    int index1 = e.getNode1() - 1;
    int index2 = e.getNode2() - 1;
    int index3 = index1 + nnodes;
    int index4 = index2 + nnodes;

    K.at(index1).at(index1) += localK.at(0).at(0);
    K.at(index1).at(index2) += localK.at(0).at(1);
    K.at(index2).at(index1) += localK.at(1).at(0);
    K.at(index2).at(index2) += localK.at(1).at(1);

    K.at(index1).at(index3) += localK.at(0).at(2);
    K.at(index1).at(index4) += localK.at(0).at(3);
    K.at(index2).at(index3) += localK.at(1).at(2);
    K.at(index2).at(index4) += localK.at(1).at(3);

    K.at(index3).at(index1) += localK.at(2).at(0);
    K.at(index3).at(index2) += localK.at(2).at(1);
    K.at(index4).at(index1) += localK.at(3).at(0);
    K.at(index4).at(index2) += localK.at(3).at(1);

}

void assemblyb(element e,Vector localb,Vector &b){
    int index1 = e.getNode1() - 1;
    int index2 = e.getNode2() - 1;

    b.at(index1) += localb.at(0);
    b.at(index2) += localb.at(1);
}

void ensamblaje(mesh &m,vector<Matrix> &localKs,vector<Vector> &localbs,Matrix &K,Vector &b){
    int nnodes = m.getSize(NODES);
    for(int i=0;i<m.getSize(ELEMENTS);i++){
        element e = m.getElement(i);
        assemblyK(e,localKs.at(i),K,nnodes);
        assemblyb(e,localbs.at(i),b);
    }
}


void applyDirichlet(mesh &m,Matrix &K,Vector &b){
    for(int i=0;i<m.getSize(DIRICHLET);i++){
        condition c = m.getCondition(i,DIRICHLET);

        int index = c.getNode1()-1;

        K.erase(K.begin()+index);
        b.erase(b.begin()+index);

        for(int row=0;row<K.size();row++){
            float cell = K.at(row).at(index);
            K.at(row).erase(K.at(row).begin()+index);
            b.at(row) += -1*c.getValue()*cell;
        }
    }
}


void calculate(Matrix &K, Vector &b, Vector &T){
    cout << "Iniciando calculo de respuesta...\n";
    Matrix Kinv;
    cout << "Calculo de inversa...\n";
    inverseMatrix(K,Kinv);
    cout << "Calculo de respuesta...\n";
    productMatrixVector(Kinv,b,T);
}
