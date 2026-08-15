pipeline {
    agent any

    environment {
        DOCKER_IMAGE = 'reaktivnii/backpipe:arm64'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build ARM64 Image') {
            steps {
                sh 'docker buildx build --platform linux/arm64 --network=host -t $DOCKER_IMAGE .'
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
