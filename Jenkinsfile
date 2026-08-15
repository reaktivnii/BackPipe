pipeline {
    agent any

    environment {
        DOCKER_IMAGE = 'reaktivnii/backpipe:arm64'
        DOCKER_CREDENTIALS = credentials('bd10aaf3-1535-4302-95e5-a114b5f46bd9')
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
                sh '''
                echo "$DOCKER_CREDENTIALS" | docker login -u reaktivnii --password-stdin

                docker push $DOCKER_IMAGE
                
                '''
            }
        }
    }
}
