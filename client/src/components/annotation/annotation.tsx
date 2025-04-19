import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import styles from './annotation.module.css';

const Annotation = () => {
  return (
    <Card>
      <CardHeader>
        <span>username</span>
        <span class={styles.annotation_date}>
          1 month ago
        </span>
      </CardHeader>
      <CardBody>
        It's common in Greek for names to be preceded by a definite article
      </CardBody>
      <CardFooter>
        --
      </CardFooter>
    </Card>
  );
};

export default Annotation;