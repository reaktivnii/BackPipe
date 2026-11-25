pipeline {
    agent any

    environment {
        DOCKER_IMAGE = 'reaktivnii/backpipe:latest'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build Images') {
            steps {
                sh 'docker buildx build --platform linux/amd64,linux/arm64 --network=host --no-cache -t $DOCKER_IMAGE .'
            }
        }

        stage('Push to Docker Hub') {
            steps {
                withCredentials([usernamePassword(
                credentialsId: 'docker-hub-creds',
                usernameVariable: 'DOCKER_USER',
                passwordVariable: 'DOCKER_PASS'
                )]) {
                sh '''
                echo "$DOCKER_PASS" | docker login -u "$DOCKER_USER" --password-stdin

                docker push $DOCKER_IMAGE
                
                '''
                }
            }
        }
    }
}
